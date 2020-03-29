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


TEST(Spleeter, TwoStems) {
  // Read wav file
  wave::File file;
//  std::string test_file("/Users/gvne/Desktop/snipet.wav");
  std::string test_file(TEST_FILE);
  file.Open(test_file, wave::kIn);
  std::error_code err;
  std::vector<float> data;
  file.Read(&data);
  auto source = Eigen::Map<spleeter::Waveform>(data.data(), 2, data.size() / 2);

  // ------------------------------
  // Spleeter !
  spleeter::Initialize(
      std::string(SPLEETER_MODELS),
      {spleeter::TwoStems, spleeter::FourStems, spleeter::FiveStems}, err);
  ASSERT_FALSE(err);
  {
    spleeter::Waveform vocals, accompaniment;
    spleeter::Split(source, &vocals, &accompaniment, err);
    ASSERT_FALSE(err);
    Write(vocals, "vocals-2stems");
    Write(accompaniment, "accompaniment-2stems");
  }
  {
    spleeter::Waveform vocals, drums, bass, other;
    spleeter::Split(source, &vocals, &drums, &bass, &other, err);
    ASSERT_FALSE(err);
    Write(vocals, "vocals-4stems");
    Write(drums, "drums-4stems");
    Write(bass, "bass-4stems");
    Write(other, "other-4stems");
  }
  {
    spleeter::Waveform vocals, drums, bass, piano, other;
    spleeter::Split(source, &vocals, &drums, &bass, &piano, &other, err);
    ASSERT_FALSE(err);
    Write(vocals, "vocals-5stems");
    Write(drums, "drums-5stems");
    Write(bass, "bass-5stems");
    Write(piano, "piano-5stems");
    Write(other, "other-5stems");
  }
}
