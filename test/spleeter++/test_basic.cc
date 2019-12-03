#include <gtest/gtest.h>
#include "spleeter++/spleeter++.h"

// ----------
// TODO: this should be moved to another file.
class Environment : public ::testing::Environment {
 public:
  virtual ~Environment() {}

  void SetUp() override {
    spleeter::Initialize(std::string(SPLEETER_ENV));
  }
};
::testing::Environment* const foo_env = ::testing::AddGlobalTestEnvironment(new Environment);
// ----------

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

