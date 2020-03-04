#ifndef ARTFF_CIRCULAR_FRAME_BUFFER_H_
#define ARTFF_CIRCULAR_FRAME_BUFFER_H_

#include <vector>
#include <complex>

namespace artff {
class CircularFrameBuffer {
public:
  CircularFrameBuffer();

  std::vector<std::complex<float> *>
  Push(const std::vector<std::complex<float> *> &data);

  void Pop(std::vector<std::complex<float> *> *data);

  void Reset();

  void Resize(uint32_t fft_size, uint8_t channel_count, uint32_t frame_count);

private:
  // Each frame_count * fft_size * channel_count
  std::vector<std::vector<std::vector<std::complex<float>>>> m_storage;
  uint32_t m_push_index;
  uint32_t m_pop_index;
};
} // namespace artff

#endif // ARTFF_CIRCULAR_FRAME_BUFFER_H_
