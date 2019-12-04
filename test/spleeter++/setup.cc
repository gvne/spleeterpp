#include <gtest/gtest.h>
#include "spleeter++/spleeter++.h"

class Environment : public ::testing::Environment {
public:
  virtual ~Environment() {}

  void SetUp() override { spleeter::Initialize(std::string(SPLEETER_ENV)); }
};

::testing::Environment *const foo_env =
    ::testing::AddGlobalTestEnvironment(new Environment);
