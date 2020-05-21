#include "artff/semaphore.h"

namespace artff {

Semaphore::Semaphore() : m_count(0) {}

void Semaphore::Notify() {
  std::lock_guard<std::mutex> lock(m_mutex);
  ++m_count;
  m_condition.notify_one();
}

void Semaphore::Wait() {
  std::unique_lock<std::mutex> lock(m_mutex);
  while (m_count > 0) {
      m_condition.wait(lock);
  }
  --m_count;
}

}  // namespace artff
