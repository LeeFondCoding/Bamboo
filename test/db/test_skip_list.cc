#include "db/SkipList.h"

#include "gtest/gtest.h"

using namespace bamboo;
using namespace db;
TEST(skiplist_test, basic_test) {
    SkipList<int, int> list{3};
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE( list.insertElement(1, 2));
    EXPECT_EQ(1, list.size());
    EXPECT_TRUE( list.insertElement(2, 3));
    EXPECT_EQ(2, list.size());
    EXPECT_TRUE(list.insertElement(3, 4));
    EXPECT_EQ(3, list.size());
    EXPECT_TRUE( list.insertElement(4, 5));
    EXPECT_EQ(4, list.size());
    EXPECT_FALSE( list.insertElement(2, 6));
    EXPECT_EQ(4, list.size());
    EXPECT_EQ(3, list.searchElement(2));
    EXPECT_EQ(2, list.searchElement(1));
    list.deleteElement(1);
    EXPECT_EQ(list.searchElement(1), nullptr);
}
int main() {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}