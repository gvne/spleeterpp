#include "spleeter/filter.h"

#include "tensorflow/core/framework/tensor_util.h"

#include "spleeter/tensor/copy.h"
#include "spleeter/model.h"
#include "spleeter/registry.h"

namespace spleeter {

Filter::Filter(SeparationType type)
    : m_type(type), m_process_length(SPLEETER_INPUT_FRAME_COUNT),
      m_frame_length(m_process_length), m_overlap_length(0),
      m_force_conservativity(false) {
  switch (type) {
  case SeparationType::TwoStems:
    m_volumes = std::vector<float>(2, 1);
    break;
  case SeparationType::FourStems:
    m_volumes = std::vector<float>(4, 1);
    break;
  case SeparationType::FiveStems:
    m_volumes = std::vector<float>(5, 1);
    break;
  }
}

void Filter::Init(std::error_code &err) {
  // stft parameters Forced by spleeter.
  // TODO: Read it from json file ?
  const auto channel_number = 2;
  const auto frame_length = 4096;
  const auto frame_step = 1024;

  rtff::AbstractFilter::Init(channel_number, frame_length,
                             frame_length - frame_step,
                             rtff::fft_window::Type::Hann, err);

  m_bundle = Registry::instance().Get(m_type);
  if (!m_bundle) {
    err = std::make_error_code(std::errc::inappropriate_io_control_operation);
    return;
  }
}

void Filter::set_volume(uint8_t stem_index, float value) {
  m_volumes[stem_index] = value;
}
float Filter::volume(uint8_t stem_index) const { return m_volumes[stem_index]; }

void Filter::set_ProcessLength(uint16_t size) { m_process_length = size; }
uint16_t Filter::ProcessLength() const { return m_process_length; }

void Filter::set_FrameLength(uint16_t size) { m_frame_length = size; }
uint16_t Filter::FrameLength() const { return m_frame_length; }

void Filter::set_OverlapLength(uint16_t size) { m_overlap_length = size; }
uint16_t Filter::OverlapLength() const { return m_overlap_length; }

void Filter::set_ForceConservativity(bool value) {
  m_force_conservativity = value;
}
bool Filter::ForceConservativity() const { return m_force_conservativity; }

uint32_t Filter::FrameLatency() const {
  return rtff::AbstractFilter::FrameLatency() + SpleeterFrameLatency();
}

uint32_t Filter::SpleeterFrameLatency() const {
  return ProcessLength() - ((ProcessLength() - FrameLength()) / 2);
}

void Filter::PrepareToPlay() {
  rtff::AbstractFilter::PrepareToPlay();
  const auto half_frame_length = fft_size() / 2 + 1;
  const auto stem_count = m_volumes.size();
  // Initialize the buffers
  // -- inputs
  tensorflow::TensorShape shape{ProcessLength() + OverlapLength(),
                                half_frame_length, 2};
  m_network_input = tensorflow::Tensor(tensorflow::DT_COMPLEX64, shape);
  m_previous_network_input =
      tensorflow::Tensor(tensorflow::DT_COMPLEX64, shape);

  // -- outputs
  m_network_result.clear();
  m_previous_network_result.clear();
  for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
    m_previous_network_result.push_back(
        tensorflow::Tensor(tensorflow::DT_FLOAT, shape));
  }

  // -- We also need pre-allocated data to retreive a single frame
  // => For overlap
  m_mask_vec_data.clear();
  m_previous_mask_vec_data.clear();
  m_mask_sum_vec_data.clear();
  m_mask_data.clear();
  m_previous_mask_data.clear();
  m_mask_sum_data.clear();
  for (auto c = 0; c < channel_count(); c++) {
    m_mask_vec_data.emplace_back(std::vector<float>(half_frame_length, 0));
    m_mask_data.push_back(m_mask_vec_data[c].data());
    m_previous_mask_vec_data.emplace_back(
        std::vector<float>(half_frame_length, 0));
    m_previous_mask_data.push_back(m_previous_mask_vec_data[c].data());
    m_mask_sum_vec_data.emplace_back(std::vector<float>(half_frame_length, 0));
    m_mask_sum_data.push_back(m_mask_sum_vec_data[c].data());
  }
  // => For masking / volumes
  m_masks_vec_data.clear();
  m_masks_data.clear();
  for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
    std::vector<std::vector<float>> single_mask_vec_data;
    std::vector<float *> single_mask_data;
    for (auto c = 0; c < channel_count(); c++) {
      single_mask_vec_data.emplace_back(
          std::vector<float>(half_frame_length, 0));
      single_mask_data.push_back(single_mask_vec_data[c].data());
    }
    m_masks_vec_data.emplace_back(std::move(single_mask_vec_data));
    m_masks_data.emplace_back(std::move(single_mask_data));
  }

  // Also reset the current frame index
  m_frame_index = 0;
}

void Filter::ProcessTransformedBlock(std::vector<std::complex<float> *> data,
                                     uint32_t size) {
  // --------------------------------
  // Set the frame into the input
  const auto stem_count = m_previous_network_result.size();
  auto network_input_frame_index =
      m_frame_index + (ProcessLength() - SpleeterFrameLatency());
  tensor::SetFrame(&m_network_input, network_input_frame_index, data);
  // --------------------------------

  // --------------------------------
  // Compute the output
  // -- get the right frame
  tensor::GetFrame(&data, network_input_frame_index, m_previous_network_input);
  // -- Get each stem mask data
  for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
    tensor::GetFrame(&(m_masks_data[stem_index]), network_input_frame_index,
                     m_previous_network_result[stem_index]);
  }
  // -- Apply a mask that is the sum of each masks * volume
  for (auto channel_index = 0; channel_index < data.size(); channel_index++) {
    // force conservativity
    // TODO: this does not work and I don't get why... FIXME !!
    if (ForceConservativity()) {
      // -- compute the mask sum (to make it conservative if asked)
      Eigen::Map<Eigen::VectorXf> mask_sum(m_mask_sum_data[channel_index],
                                           size);
      mask_sum.array() *= 0.0;
      for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
        mask_sum += Eigen::Map<Eigen::VectorXf>(
            m_masks_data[stem_index][channel_index], size);
      }
      // devide each mask by the mask sum
      for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
        Eigen::Map<Eigen::VectorXf>(m_masks_data[stem_index][channel_index],
                                    size)
            .array() /= mask_sum.array();
      }
    }

    // Apply the volumes
    Eigen::Map<Eigen::VectorXf> result_mask(m_mask_data[channel_index], size);
    result_mask.array() *= 0.0;
    for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
      Eigen::Map<Eigen::VectorXf> stem_mask(
          m_masks_data[stem_index][channel_index], size);
      result_mask.array() += stem_mask.array() * m_volumes[stem_index];
    }

    // Compute the result
    Eigen::Map<Eigen::VectorXcf> fft_frame(data[channel_index], size);
    fft_frame.array() *= result_mask.array();
  }
  // --------------------------------

  if (m_frame_index == FrameLength() - 1) {
    // --------------------------------
    // Run the extraction !
    auto status =
        m_bundle->session->Run({{"Placeholder", m_network_input}},
                               GetOutputNames(m_type), {}, &m_network_result);
    // TODO: make sure status is checked !
    //    ASSERT_TRUE(status.ok());

    // Overlap --> Update the result by adding the previous output frame and
    // devide by 2
    // TODO: use a cross fade instead of a mean to handle overlap
    for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
      for (auto overlap_frame_index = 0; overlap_frame_index < OverlapLength();
           overlap_frame_index++) {
        auto network_output_index =
            overlap_frame_index + (ProcessLength() - SpleeterFrameLatency());
        auto previous_network_output_index =
            network_output_index + FrameLength();

        tensor::GetFrame(&m_previous_mask_data, previous_network_output_index,
                         m_previous_network_result[stem_index]);
        tensor::GetFrame(&m_mask_data, network_output_index,
                         m_network_result[stem_index]);
        for (auto c = 0; c < data.size(); c++) {
          Eigen::Map<Eigen::VectorXf> mask_frame(m_mask_data[c], size);
          mask_frame +=
              Eigen::Map<Eigen::VectorXf>(m_previous_mask_data[c], size);
          mask_frame /= 2;
        }
        tensor::SetFrame(&(m_network_result[stem_index]), network_output_index,
                         m_mask_data);
      }
    }
    // TODO: this is most likely to allocate memory. Redevelop the deep copy to
    // do a simple memcpy
    for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
      m_previous_network_result[stem_index] =
          tensorflow::tensor::DeepCopy(m_network_result[stem_index]);
    }
    m_previous_network_input = tensorflow::tensor::DeepCopy(m_network_input);
    // --------------------------------

    // shift the input data of FrameLength
    for (int source_index = ProcessLength() - SpleeterFrameLatency();
         source_index < ProcessLength(); source_index++) {
      auto destination_index = source_index - FrameLength();
      if (destination_index < 0) {
        continue;
      }
      tensor::MoveFrame(m_network_input, source_index, destination_index);
    }
    m_frame_index = 0;
  } else {
    m_frame_index += 1;
  }
}

} // namespace spleeter
