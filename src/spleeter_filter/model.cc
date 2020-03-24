#include "spleeter_filter/model.h"

namespace spleeter {

std::vector<std::string> GetOutputNames(SeparationType type) {
  // Found using the command
  // $> saved_model_cli show --dir _deps/spleeter-build/exported/Xstems --all
  switch (type) {
  case TwoStems:
    return {"strided_slice_11", "strided_slice_19"};
  case FourStems:
    return {"strided_slice_11", "strided_slice_19", "strided_slice_27",
            "strided_slice_35"};
  case FiveStems:
    return {"strided_slice_16", "strided_slice_32", "strided_slice_40",
            "strided_slice_24", "strided_slice_48"};
  default:
    return {};
  }
}

} // namespace spleeter
