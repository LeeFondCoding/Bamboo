#include "base/LogFile.h"

#include "gtest/gtest.h"

using namespace bamboo;
using namespace std;

int main() {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}

const char* s[] = {"adsadadd \n", "dafafaf \n", "fafafaf \n", "afafafaf \n", "gjogiogjio\n"};
TEST(test_log_file, basic_test) {
  LogFile log("test", 10);
  log.append("hello", 5);
  log.append("nihaowodegongzhu/n", 16);
  auto c = "aiuhyigerfuiufmegjjt";
  log.append(c, strlen(c));

  for (int i = 0; i < 100; ++i) {
    log.append(s[i % 5], strlen(s[i % 5]));
  }
}

TEST(test_log_file, hard_test) {
  LogFile log("test", 10);
  for (int i = 0; i < 100000; ++i) {
    log.append(s[i % 5], strlen(s[i % 5]));
  }
}