#include "spleeter_filter/filter.h"

#include "spleeter_common/registry.h"

#include "spleeter_filter/tensor.h"
#include "spleeter_filter/model.h"
#include "spleeter_common/tf_handle.h"

namespace spleeter {

using Bundle = std::pair<TFHandlePtr<TF_Session>, TFHandlePtr<TF_Graph>>;
using BundlePtr = std::shared_ptr<Bundle>;

class Filter::Impl {
public:
  Impl() {}

  BundlePtr bundle;

  // ------------
  // internal buffers
  // -- The frame being processed
  uint32_t frame_index;
  // -- in
  TFHandlePtr<TF_Tensor> network_input;
  TFHandlePtr<TF_Tensor> previous_network_input;
  std::vector<int64_t> shapes;
  // -- out
  std::vector<TFHandlePtr<TF_Tensor>> network_result;
  std::vector<TFHandlePtr<TF_Tensor>> previous_network_result;
  // -- for single frame processing
  std::vector<std::vector<float>> mask_vec_data;
  std::vector<std::vector<float>> previous_mask_vec_data;
  std::vector<std::vector<float>> mask_sum_vec_data;
  std::vector<float *> mask_data;
  std::vector<float *> previous_mask_data;
  std::vector<float *> mask_sum_data;
  std::vector<std::vector<std::vector<float>>> masks_vec_data;
  std::vector<std::vector<float *>> masks_data;
};

Filter::Filter(SeparationType type)
    : artff::AbstractFilter(true), m_type(type),
      m_process_length(SPLEETER_INPUT_FRAME_COUNT),
      m_frame_length(m_process_length), m_overlap_length(0),
      m_force_conservativity(false), m_impl(std::make_shared<Impl>()) {
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

  m_impl->bundle = Registry::instance().Get(m_type);
  if (!m_impl->bundle) {
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
  return artff::AbstractFilter::FrameLatency() + SpleeterFrameLatency();
}

uint32_t Filter::SpleeterFrameLatency() const {
  return ProcessLength() - ((ProcessLength() - FrameLength()) / 2);
}

void Filter::PrepareToPlay() {
  artff::AbstractFilter::PrepareToPlay();
  const auto half_frame_length = fft_size() / 2 + 1;
  const auto stem_count = m_volumes.size();
  // Initialize the buffers
  // -- inputs
  m_impl->shapes = {ProcessLength() + OverlapLength(), half_frame_length, 2};
  m_impl->network_input = TensorAlloc(TF_DataType::TF_COMPLEX, m_impl->shapes);
  m_impl->previous_network_input =
      TensorAlloc(TF_DataType::TF_COMPLEX, m_impl->shapes);

  // -- outputs
  m_impl->network_result.clear();
  m_impl->previous_network_result.clear();
  for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
    m_impl->network_result.push_back(
        TensorAlloc(TF_DataType::TF_FLOAT, m_impl->shapes));
    m_impl->previous_network_result.push_back(
        TensorAlloc(TF_DataType::TF_FLOAT, m_impl->shapes));
  }

  // -- We also need pre-allocated data to retreive a single frame
  // => For overlap
  m_impl->mask_vec_data.clear();
  m_impl->previous_mask_vec_data.clear();
  m_impl->mask_sum_vec_data.clear();
  m_impl->mask_data.clear();
  m_impl->previous_mask_data.clear();
  m_impl->mask_sum_data.clear();
  for (auto c = 0; c < channel_count(); c++) {
    m_impl->mask_vec_data.emplace_back(
        std::vector<float>(half_frame_length, 0));
    m_impl->mask_data.push_back(m_impl->mask_vec_data[c].data());
    m_impl->previous_mask_vec_data.emplace_back(
        std::vector<float>(half_frame_length, 0));
    m_impl->previous_mask_data.push_back(
        m_impl->previous_mask_vec_data[c].data());
    m_impl->mask_sum_vec_data.emplace_back(
        std::vector<float>(half_frame_length, 0));
    m_impl->mask_sum_data.push_back(m_impl->mask_sum_vec_data[c].data());
  }
  // => For masking / volumes
  m_impl->masks_vec_data.clear();
  m_impl->masks_data.clear();
  for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
    std::vector<std::vector<float>> single_mask_vec_data;
    std::vector<float *> single_mask_data;
    for (auto c = 0; c < channel_count(); c++) {
      single_mask_vec_data.emplace_back(
          std::vector<float>(half_frame_length, 0));
      single_mask_data.push_back(single_mask_vec_data[c].data());
    }
    m_impl->masks_vec_data.emplace_back(std::move(single_mask_vec_data));
    m_impl->masks_data.emplace_back(std::move(single_mask_data));
  }

  // Also reset the current frame index
  m_impl->frame_index = 0;
}

void Filter::AsyncProcessTransformedBlock(
    std::vector<std::complex<float> *> data, uint32_t size) {
  // --------------------------------
  // Set the frame into the input
  const auto stem_count = m_impl->previous_network_result.size();
  auto network_input_frame_index =
      m_impl->frame_index + (ProcessLength() - SpleeterFrameLatency());
  SetFrame(m_impl->network_input, m_impl->shapes, network_input_frame_index,
           data);
  // --------------------------------

  // --------------------------------
  // Compute the output
  // -- get the right frame
  GetFrame(&data, network_input_frame_index, m_impl->previous_network_input,
           m_impl->shapes);
  // -- Get each stem mask data
  for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
    GetFrame(&(m_impl->masks_data[stem_index]), network_input_frame_index,
             m_impl->network_result[stem_index], m_impl->shapes);
  }
  // -- Apply a mask that is the sum of each masks * volume
  for (auto channel_index = 0; channel_index < data.size(); channel_index++) {
    // force conservativity
    // TODO: this does not work and I don't get why... FIXME !!
    if (ForceConservativity()) {
      // -- compute the mask sum (to make it conservative if asked)
      Eigen::Map<Eigen::VectorXf> mask_sum(m_impl->mask_sum_data[channel_index],
                                           size);
      mask_sum.array() *= 0.0;
      for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
        mask_sum += Eigen::Map<Eigen::VectorXf>(
            m_impl->masks_data[stem_index][channel_index], size);
      }
      // devide each mask by the mask sum
      for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
        Eigen::Map<Eigen::VectorXf>(
            m_impl->masks_data[stem_index][channel_index], size)
            .array() /= mask_sum.array();
      }
    }

    // Apply the volumes
    Eigen::Map<Eigen::VectorXf> result_mask(m_impl->mask_data[channel_index],
                                            size);
    result_mask.array() *= 0.0;
    for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
      Eigen::Map<Eigen::VectorXf> stem_mask(
          m_impl->masks_data[stem_index][channel_index], size);
      result_mask.array() += stem_mask.array() * m_volumes[stem_index];
    }

    // Compute the result
    Eigen::Map<Eigen::VectorXcf> fft_frame(data[channel_index], size);
    fft_frame.array() *= result_mask.array();
  }
  // --------------------------------

  if (m_impl->frame_index == FrameLength() - 1) {
    // --------------------------------
    // Run the extraction !
    auto session = m_impl->bundle->first;
    auto graph = m_impl->bundle->second;
    TF_Output input_op{TF_GraphOperationByName(graph->get(), "Placeholder"), 0};
    std::vector<TF_Tensor *> inputs = {m_impl->network_input->get()};
    std::vector<TF_Output> output_ops;
    std::vector<TF_Tensor *> outputs;
    for (const auto &output_name : GetOutputNames(m_type)) {
      TF_Output op{TF_GraphOperationByName(graph->get(), output_name.c_str()),
                   0};
      output_ops.emplace_back(op);
      outputs.push_back(nullptr);
    }

    auto status = MakeHandle(TF_NewStatus(), TF_DeleteStatus);
    TF_SessionRun(session->get(), nullptr, &input_op, inputs.data(),
                  inputs.size(), output_ops.data(), outputs.data(),
                  output_ops.size(), nullptr, 0, nullptr, status->get());
    // make sure status is checked !
    assert(TF_GetCode(status->get()) == TF_Code::TF_OK);

    // copy the outputs
    for (auto out_index = 0; out_index < outputs.size(); out_index++) {
      Copy<float>(outputs[out_index], m_impl->shapes,
                  m_impl->network_result[out_index]);
      // TODO: delete the outpus or we will get a memory leak...
      TF_DeleteTensor(outputs[out_index]);
    }

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
        GetFrame(&(m_impl->previous_mask_data), previous_network_output_index,
                 m_impl->previous_network_result[stem_index], m_impl->shapes);
        GetFrame(&(m_impl->mask_data), network_output_index,
                 m_impl->network_result[stem_index], m_impl->shapes);

        for (auto channel_index = 0; channel_index < data.size();
             channel_index++) {
          Eigen::Map<Eigen::VectorXf> mask_frame(
              m_impl->mask_data[channel_index], size);
          mask_frame += Eigen::Map<Eigen::VectorXf>(
              m_impl->previous_mask_data[channel_index], size);
          mask_frame /= 2;
        }
        SetFrame(m_impl->network_result[stem_index], m_impl->shapes,
                 network_output_index, m_impl->mask_data);
      }
    }

    // keep the output
    for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
      Copy<float>(m_impl->network_result[stem_index]->get(), m_impl->shapes,
                  m_impl->previous_network_result[stem_index]);
    }
    // And the input
    Copy<std::complex<float>>(m_impl->network_input->get(), m_impl->shapes,
                              m_impl->previous_network_input);
    // --------------------------------

    // shift the input data of FrameLength
    for (int source_index = FrameLength(); source_index < ProcessLength() + OverlapLength(); source_index++) {
      auto destination_index = source_index - FrameLength();
      MoveFrame<std::complex<float>>(m_impl->network_input, source_index,
                                     destination_index, m_impl->shapes);
    }
    m_impl->frame_index = 0;
  } else {
    m_impl->frame_index += 1;
  }
}

} // namespace spleeter
