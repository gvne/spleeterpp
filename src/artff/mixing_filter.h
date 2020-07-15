#ifndef ARTFF_MIXING_FILTER_H_
#define ARTFF_MIXING_FILTER_H_

#include "rtff/mixing_filter.h"
#include <vector>
#include <iostream>
#include <future>

#include "artff/semaphore.h"
#include "artff/buffer/circular_frame_buffer.h"
#include "artff/buffer/circular_future_buffer.h"

namespace artff {

/// The Asynct RTFF filter base class
/// This filter launches the ProcessTransformedBlock function asynchronously
/// When an fft frame needs to be processed, its computation is launched in
/// another thread and the result of the computation will be used after a few
/// frames defined by the set_extra_frame_latency parameter
///
/// This is useful when designing a filter that computes blocks of frames.
/// Indeed in that case, processing the block may take some time and will
/// block the audio thread for longer than authorized.
/// Adding some computation latency fixes the problem
class MixingFilter : public rtff::MixingFilter {
public:
  MixingFilter(uint8_t input_count, uint8_t output_count, bool sequential = false);
  void set_extra_frame_latency(uint32_t count);
  uint32_t set_extra_frame_latency() const;
  virtual uint32_t FrameLatency() const override;
  uint32_t frame_size() const;

private:
  void ProcessTransformedBlock(const std::vector<const TransformedBlock *> &inputs, const std::vector<TransformedBlock *> &outputs) override;

protected:
  virtual void PrepareToPlay() override;

  using MultichannelFrame = std::vector<std::complex<float>*>;
  using MultichannelConstFrame = std::vector<const std::complex<float>*>;
  using MultiSourceFrame = std::vector<MultichannelFrame>;
  using MultiConstSourceFrame = std::vector<MultichannelConstFrame>;

  virtual void AsyncProcessTransformedBlock(const MultiConstSourceFrame &inputs, const MultiSourceFrame &outputs) = 0;
  void SequentialAsyncProcessTransformedBlock(const MultiConstSourceFrame &inputs, const MultiSourceFrame &outputs);

private:
  bool m_sequential;
  Semaphore m_semaphore;

  uint32_t m_extra_frame_latency;

  // Storage
  std::vector<CircularFrameBuffer> m_input_circular_frame_buffer;
  std::vector<CircularFrameBuffer> m_output_circular_frame_buffer;
  CircularFutureBuffer m_circular_future;
};

} // namespace artff

#endif // ARTFF_MIXING_FILTER_H_
