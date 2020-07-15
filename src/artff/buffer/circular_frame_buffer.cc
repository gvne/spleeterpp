#include "artff/buffer/circular_frame_buffer.h"

namespace artff {
CircularFrameBuffer::CircularFrameBuffer() : m_push_index(0), m_pop_index(0) {}

std::complex<float>* CircularFrameBuffer::Push(const std::complex<float> *data) {
  auto &storage = m_storage[m_push_index];
  std::copy(data, data + storage.size(), storage.data());
  auto retval = storage.data();
  
  m_push_index += 1;
  if (m_push_index >= m_storage.size()) {
    m_push_index = 0;
  }
  return retval;
}

void CircularFrameBuffer::Pop(std::complex<float> *data) {
  auto &storage = m_storage[m_pop_index];
  std::copy(storage.data(), storage.data() + storage.size(), data);
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

void CircularFrameBuffer::Resize(uint32_t fft_size, uint32_t frame_count) {
  for (auto frame_idx = 0; frame_idx < frame_count * 2; frame_idx++) {
    m_storage.emplace_back(std::vector<std::complex<float>>(fft_size, 0));
  }
  m_push_index = 0;
  m_pop_index = frame_count;
}
} // namespace artff
