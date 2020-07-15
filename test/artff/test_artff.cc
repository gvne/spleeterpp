#include <gtest/gtest.h>
#include "wave/file.h"
#include "artff/mixing_filter.h"

class MyFilter : public artff::MixingFilter {
public:
  MyFilter() : artff::MixingFilter(1, 5) {}
private:
  void AsyncProcessTransformedBlock(const MultiConstSourceFrame &inputs, const MultiSourceFrame &outputs) override {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
      // Copy input in output
      auto in_ptr = inputs[0][channel_idx];
      for (auto out_idx = 0; out_idx < output_count_; out_idx++) {
        auto out_ptr = outputs[out_idx][channel_idx];
        std::copy(in_ptr, in_ptr + frame_size(), out_ptr);
      }
    }
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

  rtff::Waveform buffer(filter.block_size(), filter.channel_count());
  std::vector<rtff::Waveform> output;
  for (auto i = 0; i < 5; i++) {
    output.push_back(buffer);
  }

  // Run each frames
  auto multichannel_buffer_size = filter.block_size() * buffer.channel_count();
  for (uint32_t sample_idx = 0;
       sample_idx < data.size() - multichannel_buffer_size;
       sample_idx += multichannel_buffer_size) {

    float *sample_ptr = data.data() + sample_idx;
    buffer.fromInterleaved(sample_ptr);
    filter.Write(&buffer);
    filter.Read(output.data());
    output[0].toInterleaved(sample_ptr);
  }

  wave::File result_file;
  result_file.Open(std::string(OUTPUT_DIR) + "/result.wav", wave::kOut);
  result_file.set_sample_rate(44100);
  result_file.set_channel_number(2);
  result_file.Write(data);
}
