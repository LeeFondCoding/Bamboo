#include "base/TimeStamp.h"
#include "gtest/gtest.h"

using namespace bamboo;

TEST(test_ts, toString) {
  TimeStamp ts = TimeStamp::now();
  EXPECT_STREQ("1970-01-01 03:25:36.789", ts.toString().c_str());
}

int main() {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}