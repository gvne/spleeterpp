#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "wave/file.h"

TEST(Loader, Sandbox) {
  std::string model_path(SPLEETER_MODELS);
  model_path += "/2stems";

  tensorflow::SessionOptions session_options;
  tensorflow::RunOptions run_options;
  std::unordered_set<std::string> tags({"serve"});

  tensorflow::SavedModelBundle bundle;
  if (!tensorflow::LoadSavedModel(session_options, run_options, model_path, tags, &bundle).ok()) {
    std::cerr << "LoadSavedModel failed" << std::endl;
    return;
  }

  // Read wav file
  wave::File file;
  file.Open(std::string(TEST_FILE), wave::OpenMode::kIn);
  std::error_code err;
  auto data = file.Read(err);
  if (err) {
    std::cerr << "Couldn't read input file..." << std::endl;
    return;
  }

  tensorflow::Tensor input(tensorflow::DT_FLOAT, tensorflow::TensorShape({static_cast<long long>(file.frame_number()), file.channel_number()}));
  std::copy(std::begin(data), std::end(data), input.matrix<float>().data());

  std::vector<tensorflow::Tensor> output;
  if (!bundle.session->Run({{"Placeholder", input}}, {"strided_slice_13", "strided_slice_23"}, {}, &output).ok()) {
    std::cerr << "Run Failed" << std::endl;
    return;
  }

  // write data in a std::vector<float>
  std::vector<float> result_data(data.size());  // same size as input
  auto vocals = output[0].matrix<float>().data();
//  auto acompaniments = output[1].matrix<float>().data();
  std::copy(vocals, vocals + data.size(), result_data.data());

  wave::File result;
  result.Open(std::string(OUTPUT_DIR) + "/vocals.wav", wave::OpenMode::kOut);
  result.set_sample_rate(44100);
  result.set_channel_number(2);
  result.Write(result_data, err);
  if (err) {
    std::cerr << "Couldn't write the output file" << std::endl;
    return;
  }
}
