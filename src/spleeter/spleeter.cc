#include "spleeter/spleeter.h"
#include <vector>
#include "spleeter/registry.h"

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/saved_model/loader.h"

namespace spleeter {
namespace internal {

void Initialize(const std::string &path_to_model, SeparationType type) {
  tensorflow::SessionOptions session_options;
  tensorflow::RunOptions run_options;
  std::unordered_set<std::string> tags({"serve"});

  auto bundle = std::make_shared<tensorflow::SavedModelBundle>();
  // TODO: use an std::error_code to report back to caller
  if (!tensorflow::LoadSavedModel(session_options, run_options, path_to_model,
                                  tags, bundle.get())
           .ok()) {
    std::cerr << "LoadSavedModel failed" << std::endl;
    return;
  }
  Registry::instance().Register(bundle, type);
}

} // namespace internal

void Split(const Waveform &input, Waveform *vocals, Waveform *accompaniment) {
  auto bundle = Registry::instance().Get(TwoStems);

  // Initialize the input
  tensorflow::Tensor tf_input(
      tensorflow::DT_FLOAT,
      tensorflow::TensorShape({input.cols(), input.rows()}));
  
  // TODO: unsafe...
  std::copy(input.data(), input.data() + input.size(), tf_input.matrix<float>().data());

  std::vector<tensorflow::Tensor> tf_output;
  // TODO: use std::error_code to return the status
  auto status = bundle->session->Run({{"Placeholder", tf_input}},
                                     {"strided_slice_13", "strided_slice_23"},
                                     {}, &tf_output);
  
  std::vector<Waveform*> output({vocals, accompaniment});
  for (auto index = 0; index < tf_output.size(); index++) {
    output[index]->resize(input.rows(), input.cols());
    auto tf_output_ptr = tf_output[index].matrix<float>().data();
    std::copy(tf_output_ptr, tf_output_ptr + input.size(), output[index]->data());
  }
}
void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *other) {}
void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *piano, Waveform *other) {}

} // namespace spleeter
