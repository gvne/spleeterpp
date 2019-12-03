#include <gtest/gtest.h>
#include "spleeter++/spleeter++.h"

TEST(Spleeter, Basic) {
  spleeter::Initialize(std::string(SPLEETER_ENV));
  spleeter::Separate(std::string(TEST_FILE_PATH), "vocals.wav", "other.wav");
}
