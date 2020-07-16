#ifndef SPLEETER_FILTER_FILTER_H_
#define SPLEETER_FILTER_FILTER_H_

#include <system_error>
#include <mutex>

#include "artff/mixing_filter.h"
#include "spleeter_common/type.h"

namespace spleeter {

class Filter : public artff::MixingFilter {
public:
  Filter(SeparationType separation_type);

  /// Initialize the filter with Spleeter specific options
  void Init(std::error_code &err);

  // ----------------------------------------------------------------------
  // ALGORITHM PARAMETERS

  /// Set the Neural Network input size
  /// \param size
  /// \note reducing that value will reduce the latency but also reduces the
  /// temporal information and will lower the quality
  void set_ProcessLength(uint16_t size);
  uint16_t ProcessLength() const;

  /// Set the number of frames processed at a time
  /// \param size
  /// \note Always <= ProcessLength
  void set_FrameLength(uint16_t size);
  uint16_t FrameLength() const;

  /// Set the cross fade frame count between each frames. It helps reducing the
  /// inconsistency between independent processes
  /// \param size
  void set_OverlapLength(uint16_t size);
  uint16_t OverlapLength() const;

  /// Every time we run a process, we will do it on ProcessLength.
  /// However, if we decide to reduce FrameLength, it will get more CPU
  /// intensive as we will process more often but it will reduce the latency.
  /// Latency is T - (T - FrameLength) / 2
  /// If FrameLength = 1, latency is ~T/2 as we always need to process the
  /// center of the matrix. It is the one that beneficit the most of temporal
  /// information.
  uint32_t FrameLatency() const override;

  uint8_t stem_count() const;

private:
  void PrepareToPlay() override;
  void AsyncProcessTransformedBlock(const MultiConstSourceFrame &inputs,
                                    const MultiSourceFrame &outputs) override;

  uint32_t SpleeterFrameLatency() const;

private:
  SeparationType m_type;

  uint16_t m_process_length;
  uint16_t m_frame_length;
  uint16_t m_overlap_length;

  class Impl;
  std::shared_ptr<Impl> m_impl;
};

} // namespace spleeter

#endif // SPLEETER_FILTER_FILTER_H_
