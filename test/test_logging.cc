#include "base/Logging.h"
#include "gtest/gtest.h"

using namespace bamboo;
using namespace std;

int main() {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}


TEST(test_logging, basic_test) {
  
  LOG_INFO << "hello world";
  LOG_ERROR << "hello world";
  LOG_TRACE << "hello world";
  LOG_WARN << "hello world";

}