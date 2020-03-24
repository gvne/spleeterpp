#include "spleeter_filter/spleeter.h"

#include <vector>
#include "spleeter_filter/model.h"
#include "spleeter_filter/registry.h"
#include "spleeter_filter/tf_handle.h"

namespace spleeter {

void Initialize(const std::string &path_to_models,
                const std::unordered_set<SeparationType> &separation_types,
                std::error_code &err) {
  for (auto separation_type : separation_types) {
    Initialize(path_to_models, separation_type, err);
  }
}

} // namespace spleeter
