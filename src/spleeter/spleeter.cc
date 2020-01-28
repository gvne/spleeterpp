#include "spleeter/spleeter.h"
#include <vector>
#include "spleeter/model.h"

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/saved_model/loader.h"

namespace spleeter {

void Initialize(const std::string &path_to_models,
                const std::unordered_set<SeparationType> &separation_types,
                std::error_code &err) {
  for (auto separation_type : separation_types) {
    Initialize(path_to_models, separation_type, err);
  }
}

void Split(const Waveform &input, Waveform *vocals, Waveform *accompaniment,
           std::error_code &err) {
  std::vector<tensorflow::Tensor> tf_output;
  Run(input, TwoStems, &tf_output, err);
  if (err) {
    return;
  }
  SetOutput(tf_output, input.cols(), {vocals, accompaniment});
}

void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *other, std::error_code &err) {
  std::vector<tensorflow::Tensor> tf_output;
  Run(input, FourStems, &tf_output, err);
  if (err) {
    return;
  }
  SetOutput(tf_output, input.cols(), {vocals, drums, bass, other});
}

void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *piano, Waveform *other,
           std::error_code &err) {
  std::vector<tensorflow::Tensor> tf_output;
  Run(input, FiveStems, &tf_output, err);
  if (err) {
    return;
  }
  SetOutput(tf_output, input.cols(), {vocals, drums, bass, piano, other});
}

} // namespace spleeter
