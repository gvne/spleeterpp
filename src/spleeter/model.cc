#include "spleeter/model.h"
#include "spleeter/registry.h"

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/saved_model/loader.h"

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

void Initialize(const std::string &path_to_models, SeparationType type,
                std::error_code &err) {
  auto path_to_model = GetPath(path_to_models, type);

  tensorflow::SessionOptions session_options;
  tensorflow::RunOptions run_options;
  std::unordered_set<std::string> tags({"serve"});
  auto bundle = std::make_shared<tensorflow::SavedModelBundle>();
  if (!tensorflow::LoadSavedModel(session_options, run_options, path_to_model,
                                  tags, bundle.get())
           .ok()) {
    err = std::make_error_code(std::errc::io_error);
    return;
  }
  Registry::instance().Register(bundle, type);
}

void Run(const Waveform &input, SeparationType separation_type,
         std::vector<tensorflow::Tensor> *output, std::error_code &err) {
  // Find the right model
  auto bundle = Registry::instance().Get(separation_type);
  if (!bundle) {
    err = std::make_error_code(std::errc::protocol_error);
    return;
  }

  // Initialize the input
  tensorflow::Tensor tf_input(
      tensorflow::DT_FLOAT,
      tensorflow::TensorShape({input.cols(), input.rows()}));

  // TODO: Find another way to copy the input data
  std::copy(input.data(), input.data() + input.size(),
            tf_input.matrix<float>().data());

  // Run the extraction
  auto status = bundle->session->Run(
      {{"Placeholder", tf_input}}, GetOutputNames(separation_type), {}, output);
  if (!status.ok()) {
    err = std::make_error_code(std::errc::io_error);
    return;
  }
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

} // namespace spleeter
