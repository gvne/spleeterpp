#include "spleeter/spleeter.h"
#include "spleeter/model.h"

#include "spleeter_common/spleeter_common.h"

namespace spleeter {

std::vector<std::string> GetOutputNames(SeparationType type) {
  // Found using the command
  // $> saved_model_cli show --dir _deps/spleeter-build/exported/Xstems --all
  switch (type) {
  case TwoStems:
    return {"strided_slice_13", "strided_slice_23"};
  case FourStems:
    return {"strided_slice_13", "strided_slice_23", "strided_slice_33",
            "strided_slice_43"};
  case FiveStems:
    return {"strided_slice_18", "strided_slice_38", "strided_slice_48",
            "strided_slice_28", "strided_slice_58"};
  default:
    return {};
  }
}

void Split(const Waveform &input, Waveform *vocals, Waveform *accompaniment,
           std::error_code &err) {
  std::vector<TFHandlePtr<TF_Tensor>> tf_output;
  RunModel(input, TwoStems, GetOutputNames(TwoStems), &tf_output, err);
  if (err) {
    return;
  }
  SetOutput(tf_output, input.cols(), {vocals, accompaniment});
}

void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *other, std::error_code &err) {
  std::vector<TFHandlePtr<TF_Tensor>> tf_output;
  RunModel(input, FourStems, GetOutputNames(FourStems), &tf_output, err);
  if (err) {
    return;
  }
  SetOutput(tf_output, input.cols(), {vocals, drums, bass, other});
}

void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *piano, Waveform *other,
           std::error_code &err) {
  std::vector<TFHandlePtr<TF_Tensor>> tf_output;
  RunModel(input, FiveStems, GetOutputNames(FiveStems), &tf_output, err);
  if (err) {
    return;
  }
  SetOutput(tf_output, input.cols(),
                      {vocals, drums, bass, piano, other});
}

} // namespace spleeter
