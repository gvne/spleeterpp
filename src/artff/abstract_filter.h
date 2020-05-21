#ifndef ARTFF_ABSTRACT_FILTER_H_
#define ARTFF_ABSTRACT_FILTER_H_

#include "rtff/abstract_filter.h"
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
class AbstractFilter : public rtff::AbstractFilter {
public:
  AbstractFilter(bool sequential = false);
  void set_extra_frame_latency(uint32_t count);
  uint32_t set_extra_frame_latency() const;
  virtual uint32_t FrameLatency() const override;

private:
  void ProcessTransformedBlock(std::vector<std::complex<float> *> data,
                               uint32_t size) override;

protected:
  virtual void PrepareToPlay() override;
  virtual void
  AsyncProcessTransformedBlock(std::vector<std::complex<float> *> data,
                               uint32_t size) = 0;
  void SequentialAsyncProcessTransformedBlock(
      std::vector<std::complex<float>*> data, uint32_t size);

private:
  bool m_sequential;
  Semaphore m_semaphore;

  uint32_t m_extra_frame_latency;

  // Storage
  CircularFrameBuffer m_circular_frame_buffer;
  CircularFutureBuffer m_circular_future;
};

} // namespace artff

#endif // ARTFF_ABSTRACT_FILTER_H_
