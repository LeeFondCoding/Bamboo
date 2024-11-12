#include "net/Buffer.h"

#include "gtest/gtest.h"

using namespace bamboo;

TEST(buffer_test, basic_test) {
    Buffer buf;
    EXPECT_EQ(buf.readableBytes(), 0);
    EXPECT_EQ(buf.writeableBytes(), Buffer::kInitialSize);

    auto s1 = "hello, world";
    buf.append(s1); // 12
    
}


int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}