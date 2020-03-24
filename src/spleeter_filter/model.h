#ifndef SPLEETER_FILTER_MODEL_H_
#define SPLEETER_FILTER_MODEL_H_

#include <string>
#include <vector>
#include <system_error>

#include "tf_handle.h"

#include "spleeter_filter/type.h"

namespace spleeter {

std::string GetPath(const std::string &path_to_models, SeparationType type);

std::vector<std::string> GetOutputNames(SeparationType type);

void Initialize(const std::string &path_to_models, SeparationType type,
                std::error_code &err);

void Run(const Waveform &input, SeparationType separation_type,
         std::vector<TFHandlePtr<TF_Tensor>> *result, std::error_code &err);

} // namespace spleeter

#endif // SPLEETER_FILTER_MODEL_H_
