#include "spleeter_common/spleeter_common.h"

#include <vector>

#include "spleeter_common/registry.h"
#include "spleeter_common/tf_handle.h"

namespace spleeter {

std::string GetPath(const std::string &path_to_models, SeparationType type) {
  switch (type) {
  case TwoStems:
    return path_to_models + "/2stems";
  case FourStems:
    return path_to_models + "/4stems";
  case FiveStems:
    return path_to_models + "/5stems";
  default:
    return "";
  }
}

void Initialize(const std::string &path_to_models, SeparationType type,
                std::error_code &err) {
  auto path_to_model = GetPath(path_to_models, type);

  auto session_options =
      MakeHandle(TF_NewSessionOptions(), TF_DeleteSessionOptions);
  auto graph = MakeHandle(TF_NewGraph(), TF_DeleteGraph);
  auto run_options = MakeHandle(TF_NewBuffer(), TF_DeleteBuffer);
  auto meta_graph_def = MakeHandle(TF_NewBuffer(), TF_DeleteBuffer);
  auto status = MakeHandle(TF_NewStatus(), TF_DeleteStatus);
  std::vector<const char *> tags({"serve"});

  auto session_ptr = TF_LoadSessionFromSavedModel(
      session_options->get(), run_options->get(), path_to_model.c_str(),
      tags.data(), tags.size(), graph->get(), meta_graph_def->get(),
      status->get());

  if (TF_GetCode(status->get()) != TF_Code::TF_OK) {
    err = std::make_error_code(std::errc::io_error);
    return;
  }
  auto session = MakeHandle(session_ptr, SessionDeleter);
  Registry::instance().Register(std::make_shared<Bundle>(session, graph), type);
}

void Initialize(const std::string &path_to_models,
                const std::unordered_set<SeparationType> &separation_types,
                std::error_code &err) {
  for (auto separation_type : separation_types) {
    Initialize(path_to_models, separation_type, err);
  }
}

} // namespace spleeter
