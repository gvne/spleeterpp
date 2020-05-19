#include <gtest/gtest.h>
#include "wave/file.h"
#include "artff/abstract_filter.h"

class MyFilter : public artff::AbstractFilter {
private:
  void AsyncProcessTransformedBlock(std::vector<std::complex<float> *> data,
                                    uint32_t size) override {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
};

TEST(ARTFF, Basic) {
  std::error_code err;
  // read a file
  wave::File file;
  std::string test_file(TEST_FILE);
  file.Open(test_file, wave::kIn);
  std::vector<float> data;
  file.Read(&data);
  ASSERT_FALSE(err);

  MyFilter filter;
  filter.set_extra_frame_latency(20);
  filter.Init(file.channel_number(), 2048, 1024, rtff::fft_window::Type::Hann, err);
  ASSERT_FALSE(err);

  rtff::AudioBuffer buffer(filter.block_size(), filter.channel_count());

  // Run each frames
  auto multichannel_buffer_size = filter.block_size() * buffer.channel_count();
  for (uint32_t sample_idx = 0;
       sample_idx < data.size() - multichannel_buffer_size;
       sample_idx += multichannel_buffer_size) {

    float *sample_ptr = data.data() + sample_idx;
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

