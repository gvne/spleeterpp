#include "artff/buffer/circular_frame_buffer.h"

namespace artff {
CircularFrameBuffer::CircularFrameBuffer() : m_push_index(0), m_pop_index(0) {}

std::vector<std::complex<float> *>
CircularFrameBuffer::Push(const std::vector<std::complex<float> *> &data) {
  auto &storage = m_storage[m_push_index];
  std::vector<std::complex<float> *> retval;
  for (auto channel_idx = 0; channel_idx < storage.size(); channel_idx++) {
    std::copy(data[channel_idx],
              data[channel_idx] + storage[channel_idx].size(),
              storage[channel_idx].data());
    retval.push_back(storage[channel_idx].data());
  }
  m_push_index += 1;
  if (m_push_index >= m_storage.size()) {
    m_push_index = 0;
  }
  return retval;
}

void CircularFrameBuffer::Pop(std::vector<std::complex<float> *> *data) {
  auto &storage = m_storage[m_pop_index];
  for (auto channel_idx = 0; channel_idx < storage.size(); channel_idx++) {
    std::complex<float> *ptr = storage[channel_idx].data();
    auto size = storage[channel_idx].size();
    std::copy(ptr, ptr + size, data->at(channel_idx));
  }
  m_pop_index += 1;
  if (m_pop_index >= m_storage.size()) {
    m_pop_index = 0;
  }
}

void CircularFrameBuffer::Reset() {
  m_storage.clear();
  m_push_index = 0;
  m_pop_index = 0;
}

void CircularFrameBuffer::Resize(uint32_t fft_size, uint8_t channel_count,
                                 uint32_t frame_count) {
  for (auto frame_idx = 0; frame_idx < frame_count * 2; frame_idx++) {
    std::vector<std::vector<std::complex<float>>> frame;
    for (auto channel_idx = 0; channel_idx < channel_count; channel_idx++) {
      frame.emplace_back(std::vector<std::complex<float>>(fft_size, 0));
    }
    m_storage.emplace_back(std::move(frame));
  }
  m_push_index = 0;
  m_pop_index = frame_count;
}
} // namespace artff
