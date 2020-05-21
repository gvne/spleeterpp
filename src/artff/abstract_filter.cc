#include "artff/abstract_filter.h"
#include <future>

namespace artff {

AbstractFilter::AbstractFilter(bool sequential)
    : rtff::AbstractFilter(), m_sequential(sequential), m_extra_frame_latency(0) {}

void AbstractFilter::set_extra_frame_latency(uint32_t count) {
  m_extra_frame_latency = count;
}

uint32_t AbstractFilter::set_extra_frame_latency() const {
  return m_extra_frame_latency;
}

void AbstractFilter::ProcessTransformedBlock(
    std::vector<std::complex<float> *> data, uint32_t size) {
  if (m_extra_frame_latency == 0) {
    AsyncProcessTransformedBlock(data, size);
    return;
  }
  // Store data to process & start the processing
  auto stored_data = m_circular_frame_buffer.Push(data);
  std::shared_ptr<std::future<void>> f;
  if (!m_sequential) {
    f = std::make_shared<std::future<void>>(std::async(
          std::launch::async, &AbstractFilter::AsyncProcessTransformedBlock, this,
          stored_data, size));
  } else {
    m_semaphore.Wait();
    f = std::make_shared<std::future<void>>(std::async(
          std::launch::async, 
          &AbstractFilter::SequentialAsyncProcessTransformedBlock, this,
          stored_data, size));
  }
  
  m_circular_future.Push(f);

  // Wait for the desired process to finish and copy the output data
  m_circular_future.Pop();
  m_circular_frame_buffer.Pop(&data);
}

void AbstractFilter::SequentialAsyncProcessTransformedBlock(
        std::vector<std::complex<float>*> data, uint32_t size) {
  AsyncProcessTransformedBlock(data, size);
  m_semaphore.Notify();
}

void AbstractFilter::PrepareToPlay() {
  if (m_extra_frame_latency == 0) {
    return;
  }
  m_circular_frame_buffer.Reset();
  m_circular_frame_buffer.Resize(fft_size() / 2 + 1, channel_count(),
                                 m_extra_frame_latency);
  m_circular_future.Reset();
  m_circular_future.Resize(m_extra_frame_latency);
}

uint32_t AbstractFilter::FrameLatency() const {
  return rtff::AbstractFilter::FrameLatency() + m_extra_frame_latency;
}

} // namespace artff
