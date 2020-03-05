#include "spleeter/spleeter.h"
#include <vector>
#include "spleeter/registry.h"

#include "spleeter/tf_handle.h"

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/saved_model/loader.h"

namespace spleeter {
namespace internal {

std::string GetModelPath(const std::string &path_to_models,
                         SeparationType type) {
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
  auto path_to_model = GetModelPath(path_to_models, type);

  auto session_options = MakeHandle(
    TF_NewSessionOptions(), TF_DeleteSessionOptions);
  auto graph = MakeHandle(TF_NewGraph(), TF_DeleteGraph);
  auto run_options = MakeHandle(TF_NewBuffer(), TF_DeleteBuffer);
  auto meta_graph_def = MakeHandle(TF_NewBuffer(), TF_DeleteBuffer);
  auto status = MakeHandle(TF_NewStatus(), TF_DeleteStatus);
  std::vector<const char*> tags({"serve"});

  auto session_ptr = TF_LoadSessionFromSavedModel(
    session_options->get(),
    run_options->get(),
    path_to_model.c_str(),
    tags.data(),
    tags.size(),
    graph->get(),
    meta_graph_def->get(),
    status->get()
  );

  if (TF_GetCode(status->get()) == TF_Code::TF_OK) {
    err = std::make_error_code(std::errc::io_error);
    return;
  }
  auto session = MakeHandle(session_ptr, SessionDeleter);
  Registry::instance().Register(std::make_pair(session, graph), type);
}

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

void RunModel(const Waveform &input, SeparationType separation_type,
              std::vector<tensorflow::Tensor> *output, std::error_code &err) {
  // Find the right model
  auto bundle = Registry::instance().Get(separation_type);
  if (!bundle) {
    err = std::make_error_code(std::errc::protocol_error);
    return;
  }

  auto session = bundle.first;
  auto graph = bundle.second;

  // Input
  TF_Output input_op{TF_GraphOperationByName(graph->get(), "Placeholder"), 0};
  if (!input_op.oper) {
    err = std::make_error_code(std::errc::protocol_error);
    return;
  }

  // Outputs
  std::vector<TF_Output> output_ops;
  std::vector<TF_Tensor*> output;
  for (const auto& output : GetOutputNames(separation_type)) {
    output_ops.emplace_back(TF_GraphOperationByName(graph->get(), output.c_str()), 0);
    if (!output_ops[output_ops.size() - 1].oper) {
        err = std::make_error_code(std::errc::protocol_error);
        return;
    }
    output.push_back(nullptr);
  }

  TF_SessionRun(
    session->get(),
    nullptr,
    &input_op,
    inputs.data(),
    inputs.size(),
    output_ops.data(),
    outputs.data(),
    output_ops.size(),
    nullptr,
    0,
    nullptr,
    status->get());

  if (TF_GetCode(status->get()) != TF_Code::TF_OK) {
    err = std::make_error_code(std::errc::io_error);
    return;
  }

  // // Initialize the input
  // tensorflow::Tensor tf_input(
  //     tensorflow::DT_FLOAT,
  //     tensorflow::TensorShape({input.cols(), input.rows()}));
  //
  // // TODO: Find another way to copy the input data
  // std::copy(input.data(), input.data() + input.size(),
  //           tf_input.matrix<float>().data());
  //
  // // Run the extraction
  // auto status = bundle->session->Run({{"Placeholder", tf_input}},
  //                                    internal::GetOutputNames(separation_type),
  //                                    {}, output);
  // if (!status.ok()) {
  //   err = std::make_error_code(std::errc::io_error);
  //   return;
  // }
}

void SetOutput(const std::vector<tensorflow::Tensor> &tf_output,
               uint64_t frame_count, std::vector<Waveform *> output) {
  for (auto index = 0; index < tf_output.size(); index++) {
    output[index]->resize(2, frame_count); // resize the matrix
    // Find the output data pointer
    auto tf_output_ptr = tf_output[index].matrix<float>().data();
    std::copy(tf_output_ptr, tf_output_ptr + output[index]->size(),
              output[index]->data());
  }
}

} // namespace internal

void Initialize(const std::string &path_to_models,
                const std::unordered_set<SeparationType> &separation_types,
                std::error_code &err) {
  for (auto separation_type : separation_types) {
    internal::Initialize(path_to_models, separation_type, err);
  }
}

void Split(const Waveform &input, Waveform *vocals, Waveform *accompaniment,
           std::error_code &err) {
  std::vector<tensorflow::Tensor> tf_output;
  internal::RunModel(input, TwoStems, &tf_output, err);
  if (err) {
    return;
  }
  internal::SetOutput(tf_output, input.cols(), {vocals, accompaniment});
}

void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *other, std::error_code &err) {
  std::vector<tensorflow::Tensor> tf_output;
  internal::RunModel(input, FourStems, &tf_output, err);
  if (err) {
    return;
  }
  internal::SetOutput(tf_output, input.cols(), {vocals, drums, bass, other});
}

void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *piano, Waveform *other,
           std::error_code &err) {
  std::vector<tensorflow::Tensor> tf_output;
  internal::RunModel(input, FiveStems, &tf_output, err);
  if (err) {
    return;
  }
  internal::SetOutput(tf_output, input.cols(),
                      {vocals, drums, bass, piano, other});
}

} // namespace spleeter
