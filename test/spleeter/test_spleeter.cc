#include <gtest/gtest.h>
#include "wave/file.h"
#include "spleeter/spleeter.h"

TEST(Spleeter, TwoStems) {
  // Read wav file
  wave::File file;
  file.Open(std::string(TEST_FILE), wave::kIn);
  std::error_code err;
  auto data = file.Read(err);
  if (err) {
    std::cerr << "Couldn't read input file..." << std::endl;
    return;
  }
  auto source = Eigen::Map<spleeter::Waveform>(data.data(), 2, data.size() / 2);
  
  // ------------------------------
  // Spleeter !
  spleeter::Initialize<spleeter::TwoStems>(std::string(SPLEETER_MODELS));
  spleeter::Waveform vocals, accompaniment;
  spleeter::Split<spleeter::TwoStems>(source, &vocals, &accompaniment);
  // ------------------------------
  
  // Export the results
  std::vector<float> vocals_data(vocals.size());
  std::copy(vocals.data(), vocals.data() + vocals.size(), vocals_data.data());
  wave::File vocals_file;
  vocals_file.Open(std::string(OUTPUT_DIR) + "/vocals.wav", wave::kOut);
  vocals_file.set_sample_rate(file.sample_rate());
  vocals_file.set_channel_number(2);
  vocals_file.Write(vocals_data);
  
  std::vector<float> acc_data(vocals.size());
  std::copy(accompaniment.data(), accompaniment.data() + accompaniment.size(), acc_data.data());
  wave::File acc_file;
  acc_file.Open(std::string(OUTPUT_DIR) + "/accompaniment.wav", wave::kOut);
  acc_file.set_sample_rate(file.sample_rate());
  acc_file.set_channel_number(2);
  acc_file.Write(acc_data);
}
