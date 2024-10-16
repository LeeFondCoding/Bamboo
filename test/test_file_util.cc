#include "base/FileUtil.h"

#include "gtest/gtest.h"

using namespace bamboo;
using namespace std;

TEST(file_util_test, basic_test) {
  FileUtil append_file("test.txt");
  append_file.append("hello world\n", 12);
  EXPECT_EQ(12, append_file.writtenBytes());
  append_file.flush();
}

TEST(file_util_test, hard_test) {
  FileUtil append_file("test.txt");
  size_t ans = 0;
  for (int i = 0; i < 100; ++i) {
    ans += 12;
    append_file.append("hello world\n", 12);
    EXPECT_EQ(ans, append_file.writtenBytes());
  }
  append_file.flush();
}

int main() {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}