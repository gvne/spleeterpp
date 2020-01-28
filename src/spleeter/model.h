#ifndef SPLEETER_MODEL_H_
#define SPLEETER_MODEL_H_

#include <string>
#include <vector>
#include <system_error>

#include "tensorflow/core/framework/tensor.h"

#include "spleeter/type.h"

namespace spleeter {

std::string GetPath(const std::string &path_to_models, SeparationType type);

std::vector<std::string> GetOutputNames(SeparationType type);

void Initialize(const std::string &path_to_models, SeparationType type,
                std::error_code &err);

void Run(const Waveform &input, SeparationType separation_type,
         std::vector<tensorflow::Tensor> *output, std::error_code &err);

void SetOutput(const std::vector<tensorflow::Tensor> &tf_output,
               uint64_t frame_count, std::vector<Waveform *> output);

} // namespace spleeter

#endif // SPLEETER_MODEL_H_
