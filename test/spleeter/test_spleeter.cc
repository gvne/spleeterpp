#include <gtest/gtest.h>
#include "wave/file.h"
#include "spleeter/spleeter.h"

#include "spleeter/model.h"
#include "spleeter/registry.h"

#include "rtff/filter.h"

void Write(const spleeter::Waveform& data, const std::string& name) {
  std::vector<float> vec_data(data.size());
  std::copy(data.data(), data.data() + data.size(), vec_data.data());
  wave::File file;
  file.Open(std::string(OUTPUT_DIR) + "/" + name + ".wav", wave::kOut);
  file.set_sample_rate(44100);
  file.set_channel_number(2);
  file.Write(vec_data);
}

TEST(Spleeter, Spectrogram) {
  std::error_code err;
  
  // read a file
  wave::File file;
  std::string test_file(TEST_FILE);
//  std::string test_file("/Users/gvne/Desktop/snipet.wav");
  file.Open(test_file, wave::kIn);
  auto data = file.Read(err);
  ASSERT_FALSE(err);
  
  // 60sec 44.1KHz
//  const auto sample_count = 44100 * 60;
  const auto channel_number = 2;
  // stft parameters
  const auto frame_length = 4096;
  const auto half_frame_length = frame_length / 2 + 1;
  const auto frame_step = 1024;
  
  // initialize the rtff filter
  rtff::Filter filter;
  const auto block_size = 2048;
  filter.Init(channel_number, frame_length, frame_length - frame_step, rtff::fft_window::Type::Hann, err);
  ASSERT_FALSE(err);
  filter.set_block_size(block_size);
  rtff::AudioBuffer buffer(block_size, channel_number);

//  const auto frame_count = sample_count / frame_step;
//  Eigen::MatrixXcf left = Eigen::MatrixXcf::Random(half_frame_length, frame_count);
//  Eigen::MatrixXcf right = Eigen::MatrixXcf::Random(half_frame_length, frame_count);
  
  const auto separation_type = spleeter::TwoStems;

  spleeter::Initialize(std::string(SPLEETER_MODELS), separation_type, err);
  ASSERT_FALSE(err);
  
  // Find the right model
  auto bundle = spleeter::Registry::instance().Get(separation_type);
  ASSERT_TRUE(bundle);
  
  // Initialize the input
  std::vector<tensorflow::Tensor> output;
  tensorflow::Tensor tf_input(
      tensorflow::DT_COMPLEX64,
      tensorflow::TensorShape({1, half_frame_length, 2}));
  
  filter.execute = [bundle, &tf_input, &output](std::vector<std::complex<float>*> data, uint32_t size) {
    auto flatten = tf_input.flat<std::complex<float>>();
    auto flatten_ptr = flatten.data();
    std::copy(data[0], data[0] + size, flatten_ptr);
    std::copy(data[1], data[1] + size, flatten_ptr + size);
    
    auto status = bundle->session->Run(
        {{"Placeholder", tf_input}}, GetOutputNames(separation_type), {}, &output);
    ASSERT_TRUE(status.ok());
    
    // Apply the output mask
    auto mask_size = output[0].dim_size(1);
    Eigen::Map<Eigen::VectorXf> left_mask(output[0].flat<float>().data(), mask_size);
    Eigen::Map<Eigen::VectorXf> right_mask(output[0].flat<float>().data() + mask_size, mask_size);

    Eigen::Map<Eigen::VectorXcf> left_channel(data[0], mask_size);
    Eigen::Map<Eigen::VectorXcf> right_channel(data[1], mask_size);
    
    left_channel.array() *= left_mask.array();
    right_channel.array() *= right_mask.array();
    
    output.clear();
  };
  
  // Run each frames
  auto multichannel_buffer_size = block_size * channel_number;
  for (uint32_t sample_idx = 0;
     sample_idx < data.size() - multichannel_buffer_size;
     sample_idx += multichannel_buffer_size) {
    std::cout << sample_idx << " " << data.size() << std::endl;
    // process the input buffer
    float* sample_ptr = data.data() + sample_idx;

    buffer.fromInterleaved(sample_ptr);
    filter.ProcessBlock(&buffer);
    buffer.toInterleaved(sample_ptr);
  }
  
  
  wave::File result_file;
  result_file.Open(std::string(OUTPUT_DIR) + "/result.wav", wave::kOut);
  result_file.set_sample_rate(44100);
  result_file.set_channel_number(2);
  result_file.Write(data);
}

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
