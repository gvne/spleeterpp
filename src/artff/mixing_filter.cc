#include "artff/mixing_filter.h"
#include <future>

namespace artff {

MixingFilter::MixingFilter(uint8_t input_count, uint8_t output_count, bool sequential)
    : rtff::MixingFilter(input_count, output_count), m_sequential(sequential), m_extra_frame_latency(0) {}

void MixingFilter::set_extra_frame_latency(uint32_t count) {
  m_extra_frame_latency = count;
}

uint32_t MixingFilter::set_extra_frame_latency() const {
  return m_extra_frame_latency;
}

void MixingFilter::ProcessTransformedBlock(const std::vector<const TransformedBlock *> &inputs, const std::vector<TransformedBlock *> &outputs) {
  // Store data to process
  std::vector<std::vector<const std::complex<float>*>> input_data;
  std::vector<std::vector<std::complex<float>*>> output_data;

  for (auto in_idx = 0; in_idx < input_count_; in_idx++) {
    input_data.push_back(std::vector<const std::complex<float>*>());
    for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
      auto ptr = m_input_circular_frame_buffer[channel_count() * in_idx + channel_idx].Push(inputs[in_idx]->channel(channel_idx));
      input_data[in_idx].push_back(ptr);
    }
  }
  for (auto out_idx = 0; out_idx < output_count_; out_idx++) {
    output_data.push_back(std::vector<std::complex<float>*>());
    for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
      auto ptr = m_output_circular_frame_buffer[channel_count() * out_idx + channel_idx].Push(outputs[out_idx]->channel(channel_idx));
      output_data[out_idx].push_back(ptr);
    }
  }

  // start the processing
  std::shared_ptr<std::future<void>> f;
  if (!m_sequential) {
    f = std::make_shared<std::future<void>>(std::async(
          std::launch::async, &MixingFilter::AsyncProcessTransformedBlock, this,
          input_data, output_data));
  } else {
    m_semaphore.Wait();
    f = std::make_shared<std::future<void>>(std::async(
          std::launch::async,
          &MixingFilter::SequentialAsyncProcessTransformedBlock, this,
          input_data, output_data));
  }

  m_circular_future.Push(f);

  // Wait for the desired process to finish and copy the output data
  m_circular_future.Pop();
  // Read from output
  for (auto out_idx = 0; out_idx < output_count_; out_idx++) {
    output_data.push_back(std::vector<std::complex<float>*>());
    for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
      m_output_circular_frame_buffer[channel_count() * out_idx + channel_idx].Pop(outputs[out_idx]->channel(channel_idx));
    }
  }
}

void MixingFilter::SequentialAsyncProcessTransformedBlock(const MultiConstSourceFrame &inputs, const MultiSourceFrame &outputs) {
  AsyncProcessTransformedBlock(inputs, outputs);
  m_semaphore.Notify();
}

void MixingFilter::PrepareToPlay() {
  m_input_circular_frame_buffer.clear();
  m_output_circular_frame_buffer.clear();
  for (auto idx = 0; idx < input_count_ * channel_count(); idx++) {
    CircularFrameBuffer buffer;
    buffer.Resize(frame_size(), m_extra_frame_latency);
    m_input_circular_frame_buffer.emplace_back(std::move(buffer));
  }
  for (auto idx = 0; idx < output_count_ * channel_count(); idx++) {
    CircularFrameBuffer buffer;
    buffer.Resize(frame_size(), m_extra_frame_latency);
    m_output_circular_frame_buffer.emplace_back(std::move(buffer));
  }
  m_circular_future.Reset();
  m_circular_future.Resize(m_extra_frame_latency);
}

uint32_t MixingFilter::FrameLatency() const {
  return rtff::MixingFilter::FrameLatency() + m_extra_frame_latency;
}

uint32_t MixingFilter::frame_size() const {
  return fft_size() / 2 + 1;
}

} // namespace artff
