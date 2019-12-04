#include <gtest/gtest.h>
#include "spleeter++/spleeter++.h"


TEST(Spleeter, TwoStems) {
  spleeter::Separate(std::string(TEST_FILE_PATH), "vocals.wav", "other.wav");
}

TEST(Spleeter, FourStems) {
  spleeter::Separate(std::string(TEST_FILE_PATH), "vocals.wav", "drums.wav",
                     "bass.wav", "other.wav");
}

TEST(Spleeter, FiveStems) {
  spleeter::Separate(std::string(TEST_FILE_PATH), "vocals.wav", "drums.wav",
                     "bass.wav", "piano.wav", "other.wav");
}
