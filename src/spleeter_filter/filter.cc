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
  TensorPtr<std::complex<float>> network_input;
  TensorPtr<std::complex<float>> previous_network_input;
  // -- out
  std::vector<TensorPtr<float>> network_result;
  std::vector<TensorPtr<float>> previous_network_result;
  // -- for single frame processing
  TensorFramePtr<float> mask;
  TensorFramePtr<float> previous_mask;
};

uint8_t OutputCount(SeparationType type) {
  switch (type) {
  case SeparationType::TwoStems:
    return 2;
  case SeparationType::FourStems:
    return 4;
  case SeparationType::FiveStems:
    return 5;
  }
}

Filter::Filter(SeparationType type)
    : artff::MixingFilter(1, OutputCount(type), true), m_type(type),
      m_process_length(SPLEETER_INPUT_FRAME_COUNT),
      m_frame_length(m_process_length), m_overlap_length(0),
      m_impl(std::make_shared<Impl>()) {}

void Filter::Init(std::error_code &err) {
  // stft parameters Forced by spleeter.
  // TODO: Read it from json file ?
  const auto channel_number = 2;
  const auto frame_length = 4096;
  const auto frame_step = 1024;

  rtff::MixingFilter::Init(channel_number, frame_length,
                           frame_length - frame_step,
                           rtff::fft_window::Type::Hann, err);

  m_impl->bundle = Registry::instance().Get(m_type);
  if (!m_impl->bundle) {
    err = std::make_error_code(std::errc::inappropriate_io_control_operation);
    return;
  }
}

void Filter::set_ProcessLength(uint16_t size) { m_process_length = size; }
uint16_t Filter::ProcessLength() const { return m_process_length; }

void Filter::set_FrameLength(uint16_t size) { m_frame_length = size; }
uint16_t Filter::FrameLength() const { return m_frame_length; }

void Filter::set_OverlapLength(uint16_t size) { m_overlap_length = size; }
uint16_t Filter::OverlapLength() const { return m_overlap_length; }

uint32_t Filter::FrameLatency() const {
  return artff::MixingFilter::FrameLatency() + SpleeterFrameLatency();
}

uint32_t Filter::SpleeterFrameLatency() const {
  return ProcessLength() - ((ProcessLength() - FrameLength()) / 2);
}

uint8_t Filter::stem_count() const { return output_count_; }

void Filter::PrepareToPlay() {
  artff::MixingFilter::PrepareToPlay();
  // Initialize the buffers
  // -- inputs
  std::vector<int64_t> shapes = {ProcessLength() + OverlapLength(),
                                 frame_size(), 2};
  m_impl->network_input = std::make_shared<Tensor<std::complex<float>>>(shapes);
  m_impl->previous_network_input =
      std::make_shared<Tensor<std::complex<float>>>(shapes);

  // -- outputs
  m_impl->network_result.clear();
  m_impl->previous_network_result.clear();
  for (auto stem_index = 0; stem_index < stem_count(); stem_index++) {
    m_impl->network_result.emplace_back(
        std::make_shared<Tensor<float>>(shapes));
    m_impl->previous_network_result.emplace_back(
        std::make_shared<Tensor<float>>(shapes));
  }

  // -- We also need pre-allocated data to retreive a single frame
  // => For overlap
  m_impl->mask =
      std::make_shared<TensorFrame<float>>(channel_count(), frame_size());
  m_impl->previous_mask =
      std::make_shared<TensorFrame<float>>(channel_count(), frame_size());

  // Also reset the current frame index
  m_impl->frame_index = 0;
}

void Filter::AsyncProcessTransformedBlock(const MultiConstSourceFrame &inputs,
                                          const MultiSourceFrame &outputs) {
  // --------------------------------
  // Set the frame into the input
  auto network_input_frame_index =
      m_impl->frame_index + (ProcessLength() - SpleeterFrameLatency());
  m_impl->network_input->SetFrame(network_input_frame_index, inputs[0]);
  // --------------------------------

  // --------------------------------
  // Compute the output
  // TODO: conservativity should be applied on the entire result
  for (auto stem_index = 0; stem_index < stem_count(); stem_index++) {
    // Get the neural network result
    auto mask_data = m_impl->mask->data();
    m_impl->network_result[stem_index]->GetFrame(network_input_frame_index,
                                                 mask_data);
    // And the input frame
    m_impl->previous_network_input->GetFrame(network_input_frame_index,
                                             outputs[stem_index]);
    // Apply the result to the frame
    for (auto channel_index = 0; channel_index < channel_count();
         channel_index++) {
      Eigen::Map<Eigen::VectorXcf> frame(outputs[stem_index][channel_index],
                                         frame_size());
      Eigen::Map<Eigen::VectorXf> mask(mask_data[channel_index], frame_size());
      frame.array() *= mask.array();
    }
  }

  // --------------------------------

  if (m_impl->frame_index == FrameLength() - 1) {
    // --------------------------------
    // Run the extraction !
    // TODO: the process allocates memory. Find a way to avoid that
    auto session = m_impl->bundle->first;
    auto graph = m_impl->bundle->second;
    TF_Output input_op{TF_GraphOperationByName(graph->get(), "Placeholder"), 0};
    std::vector<TF_Tensor *> inputs = {m_impl->network_input->ptr()};
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
      m_impl->network_result[out_index]->Copy(outputs[out_index]);
      // delete the outpus to avoid memory leaks
      TF_DeleteTensor(outputs[out_index]);
    }

    // --------------------------------
    // Overlap --> Update the result by adding the previous output frame and
    // divide by 2
    // TODO: use a cross fade instead of a mean to handle overlap
    for (auto stem_index = 0; stem_index < stem_count(); stem_index++) {
      for (auto overlap_frame_index = 0; overlap_frame_index < OverlapLength();
           overlap_frame_index++) {
        auto network_output_index =
            overlap_frame_index + (ProcessLength() - SpleeterFrameLatency());
        auto previous_network_output_index =
            network_output_index + FrameLength();

        auto mask_data = m_impl->mask->data();
        auto previous_mask_data = m_impl->previous_mask->data();

        m_impl->previous_network_result[stem_index]->GetFrame(
            previous_network_output_index, previous_mask_data);
        m_impl->network_result[stem_index]->GetFrame(network_output_index,
                                                     mask_data);

        for (auto channel_index = 0; channel_index < channel_count();
             channel_index++) {
          Eigen::Map<Eigen::VectorXf> mask_frame(mask_data[channel_index],
                                                 frame_size());
          Eigen::Map<Eigen::VectorXf> previous_mask_frame(
              previous_mask_data[channel_index], frame_size());

          mask_frame += previous_mask_frame;
          mask_frame /= 2;
        }
        m_impl->network_result[stem_index]->SetFrame(
            network_output_index, m_impl->mask->const_data());
      }
    }

    // keep the output to handle the next overlap
    for (auto stem_index = 0; stem_index < stem_count(); stem_index++) {
      m_impl->previous_network_result[stem_index]->Copy(
          m_impl->network_result[stem_index]->ptr());
    }
    // And the input
    m_impl->previous_network_input->Copy(m_impl->network_input->ptr());

    // shift the input data of FrameLength
    for (int source_index = FrameLength();
         source_index < ProcessLength() + OverlapLength(); source_index++) {
      auto destination_index = source_index - FrameLength();
      m_impl->network_input->MoveFrame(source_index, destination_index);
    }
    m_impl->frame_index = 0;
  } else {
    m_impl->frame_index += 1;
  }
}

} // namespace spleeter
