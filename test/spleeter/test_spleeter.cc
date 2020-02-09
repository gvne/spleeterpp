#include <gtest/gtest.h>
#include "wave/file.h"
#include "spleeter/spleeter.h"

#include "spleeter/model.h"
#include "spleeter/registry.h"

TEST(Spleeter, Spectrogram) {
  std::error_code err;
  
  // 60sec 44.1KHz
  const auto sample_count = 44100 * 60;
  // stft parameters
  const auto frame_length = 4096;
  const auto half_frame_length = frame_length / 2 + 1;
  const auto frame_step = 1024;

  const auto frame_count = sample_count / frame_step;
  Eigen::MatrixXcf left = Eigen::MatrixXcf::Random(half_frame_length, frame_count);
  Eigen::MatrixXcf right = Eigen::MatrixXcf::Random(half_frame_length, frame_count);
  
  const auto separation_type = spleeter::TwoStems;
  std::vector<tensorflow::Tensor> output;

  spleeter::Initialize(std::string(SPLEETER_MODELS), separation_type, err);
  ASSERT_FALSE(err);
  
  
  // Find the right model
  auto bundle = spleeter::Registry::instance().Get(separation_type);
  ASSERT_TRUE(bundle);
  
  // Initialize the input
  tensorflow::Tensor tf_input(
      tensorflow::DT_COMPLEX64,
      tensorflow::TensorShape({frame_count, half_frame_length, 2}));

  // TODO: Find another way to copy the input data
//  std::copy(input.data(), input.data() + input.size(),
//            tf_input.matrix<float>().data());

  // Run the extraction
  auto status = bundle->session->Run(
      {{"Placeholder", tf_input}}, GetOutputNames(separation_type), {}, &output);
  ASSERT_TRUE(status.ok());
}

// void Write(const spleeter::Waveform& data, const std::string& name) {
//  std::vector<float> vec_data(data.size());
//  std::copy(data.data(), data.data() + data.size(), vec_data.data());
//  wave::File file;
//  file.Open(std::string(OUTPUT_DIR) + "/" + name + ".wav", wave::kOut);
//  file.set_sample_rate(44100);
//  file.set_channel_number(2);
//  file.Write(vec_data);
//}
//
//
// TEST(Spleeter, TwoStems) {
//  // Read wav file
//  wave::File file;
//  file.Open(std::string(TEST_FILE), wave::kIn);
//  std::error_code err;
//  auto data = file.Read(err);
//  ASSERT_FALSE(err);
//  auto source = Eigen::Map<spleeter::Waveform>(data.data(), 2, data.size() /
//  2);
//
//  // ------------------------------
//  // Spleeter !
//  spleeter::Initialize(
//      std::string(SPLEETER_MODELS),
//      {spleeter::TwoStems, spleeter::FourStems, spleeter::FiveStems}, err);
//  ASSERT_FALSE(err);
//  {
//    spleeter::Waveform vocals, accompaniment;
//    spleeter::Split(source, &vocals, &accompaniment, err);
//    ASSERT_FALSE(err);
//    Write(vocals, "vocals-2stems");
//    Write(accompaniment, "accompaniment-2stems");
//  }
//  {
//    spleeter::Waveform vocals, drums, bass, other;
//    spleeter::Split(source, &vocals, &drums, &bass, &other, err);
//    ASSERT_FALSE(err);
//    Write(vocals, "vocals-4stems");
//    Write(drums, "drums-4stems");
//    Write(bass, "bass-4stems");
//    Write(other, "other-4stems");
//  }
//  {
//    spleeter::Waveform vocals, drums, bass, piano, other;
//    spleeter::Split(source, &vocals, &drums, &bass, &piano, &other, err);
//    ASSERT_FALSE(err);
//    Write(vocals, "vocals-5stems");
//    Write(drums, "drums-5stems");
//    Write(bass, "bass-5stems");
//    Write(piano, "piano-5stems");
//    Write(other, "other-5stems");
//  }
//}
