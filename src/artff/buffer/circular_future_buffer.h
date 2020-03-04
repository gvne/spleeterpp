#ifndef ARTFF_CIRCULAR_FUTURE_BUFFER_H_
#define ARTFF_CIRCULAR_FUTURE_BUFFER_H_

#include <vector>
#include <future>
#include <memory>

namespace artff {
class CircularFutureBuffer {
public:
  CircularFutureBuffer();
  void Reset();
  void Resize(uint32_t count);
  void Push(std::shared_ptr<std::future<void>> f);
  void Pop();

private:
  std::vector<std::shared_ptr<std::future<void>>> m_storage;
  uint32_t m_push_index;
  uint32_t m_pop_index;
};
}  // namespace artff

#endif  // ARTFF_CIRCULAR_FUTURE_BUFFER_H_
