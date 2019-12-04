#include <gtest/gtest.h>
#include "spleeter++/spleeter++.h"


TEST(Spleeter, TwoStems) {
  spleeter::Separate(
    std::string(TEST_FILE_PATH),
    std::string(OUTPUT_DIR) + "/2stems_vocals.wav",
    std::string(OUTPUT_DIR) + "/2stems_other.wav"
  );
}

TEST(Spleeter, FourStems) {
  spleeter::Separate(
    std::string(TEST_FILE_PATH),
    std::string(OUTPUT_DIR) + "/4stems_vocals.wav",
    std::string(OUTPUT_DIR) + "/4stems_drums.wav",
    std::string(OUTPUT_DIR) + "/4stems_bass.wav",
    std::string(OUTPUT_DIR) + "/4stems_other.wav"
  );
}

TEST(Spleeter, FiveStems) {
  spleeter::Separate(
    std::string(TEST_FILE_PATH),
    std::string(OUTPUT_DIR) + "/5stems_vocals.wav",
    std::string(OUTPUT_DIR) + "/5stems_drums.wav",
    std::string(OUTPUT_DIR) + "/5stems_bass.wav",
    std::string(OUTPUT_DIR) + "/5stems_piano.wav",
    std::string(OUTPUT_DIR) + "/5stems_other.wav"
  );
}
