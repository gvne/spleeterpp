#ifndef SPLEETER_MODEL_H_
#define SPLEETER_MODEL_H_

#include <vector>

#include "spleeter_common/tf_handle.h"
#include "spleeter_common/type.h"

namespace spleeter {
  
  void RunModel(const Waveform &input, SeparationType separation_type,
                const std::vector<std::string>& output_names,
                std::vector<TFHandlePtr<TF_Tensor>> *result,
                std::error_code &err);
  void SetOutput(const std::vector<TFHandlePtr<TF_Tensor>> &tf_output,
                 uint64_t frame_count, std::vector<Waveform *> output);

}  // namespace spleeter

#endif  // SPLEETER_MODEL_H_
