#include <gtest/gtest.h>

#include "wave/file.h"

#include "spleeter/spleeter.h"

void Write(const spleeter::Waveform& data, const std::string& name) {
  std::vector<float> vec_data(data.size());
  std::copy(data.data(), data.data() + data.size(), vec_data.data());
  wave::File file;
  file.Open(std::string(OUTPUT_DIR) + "/" + name + ".wav", wave::kOut);
  file.set_sample_rate(44100);
  file.set_channel_number(2);
  file.Write(vec_data);
}

TEST(Spleeter, Filter) {
  std::error_code err;

  // read a file
  wave::File file;
//  std::string test_file(TEST_FILE);
  std::string test_file("/Users/gvne/Desktop/snipet.wav");
  file.Open(test_file, wave::kIn);
  auto data = file.Read(err);
  ASSERT_FALSE(err);

  // Initialize the filter
  const auto separation_type = spleeter::FourStems;
  spleeter::Initialize(std::string(SPLEETER_MODELS), {separation_type}, err);
  ASSERT_FALSE(err);
  spleeter::Filter filter(separation_type);
  filter.Init(err);
  filter.set_volume(0, 1.0);
  filter.set_volume(1, 0.0);
  filter.set_volume(2, 0.0);
  filter.set_volume(3, 0.0);
  ASSERT_FALSE(err);

  filter.set_OverlapLength(2);
  filter.set_FrameLength(filter.ProcessLength() - filter.OverlapLength());

  // Initialize the audio buffer
  const auto block_size = 2048;
  filter.set_block_size(block_size);
  rtff::AudioBuffer buffer(block_size, filter.channel_count());

  // Run each frames
  auto multichannel_buffer_size = block_size * buffer.channel_count();
  for (uint32_t sample_idx = 0;
     sample_idx < data.size() - multichannel_buffer_size;
     sample_idx += multichannel_buffer_size) {

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
