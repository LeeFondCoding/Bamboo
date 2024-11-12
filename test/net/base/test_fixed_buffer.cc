#include "base/LogStream.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace bamboo;
using namespace std;

int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

TEST(test_fixed_buffer, basic_test) {
    FixedBuffer<64> buffer;
    buffer.append("hello", 5);
    buffer.append("world", 5);
    EXPECT_EQ(buffer.length(), 10);
    EXPECT_EQ(buffer.avail(), 54);
    cout << buffer.data() <<endl;
    buffer.bzero();
    cout << buffer.data() <<endl;
}

const char* test_data[] = {"hello ", "world ", "nihao ", "shijie ", "byebye "};
TEST(test_fixed_buffer, hard_test) {
    FixedBuffer<64> buffer;
    for (int i = 0; i < 20; ++i) {
        buffer.append(test_data[i % 5], strlen(test_data[i % 5]));
        cout << buffer.data() <<endl;
    }
}
