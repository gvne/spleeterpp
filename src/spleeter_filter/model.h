#ifndef SPLEETER_FILTER_MODEL_H_
#define SPLEETER_FILTER_MODEL_H_

#include <string>
#include <vector>
#include <system_error>

#include "spleeter_common/tf_handle.h"
#include "spleeter_common/type.h"

namespace spleeter {

std::string GetPath(const std::string &path_to_models, SeparationType type);

std::vector<std::string> GetOutputNames(SeparationType type);

} // namespace spleeter

#endif // SPLEETER_FILTER_MODEL_H_
