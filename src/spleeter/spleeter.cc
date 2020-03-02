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

} // namespace spleeter
