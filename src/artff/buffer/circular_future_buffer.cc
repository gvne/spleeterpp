#include "artff/buffer/circular_future_buffer.h"

namespace artff {
CircularFutureBuffer::CircularFutureBuffer()
    : m_push_index(0), m_pop_index(0) {}

void CircularFutureBuffer::Reset() {
  m_storage.clear();
  m_push_index = 0;
  m_pop_index = 0;
}

void CircularFutureBuffer::Resize(uint32_t count) {
  m_storage.clear();
  for (auto idx = 0; idx < count * 2; idx++) {
    m_storage.push_back(nullptr);
  }
  m_push_index = 0;
  m_pop_index = count;
}

void CircularFutureBuffer::Push(std::shared_ptr<std::future<void>> f) {
  m_storage[m_push_index] = f;
  m_push_index += 1;
  if (m_push_index >= m_storage.size()) {
    m_push_index = 0;
  }
}

void CircularFutureBuffer::Pop() {
  if (m_storage[m_pop_index]) {
    m_storage[m_pop_index]->wait();
  }
  m_pop_index += 1;
  if (m_pop_index >= m_storage.size()) {
    m_pop_index = 0;
  }
}
} // namespace artff
