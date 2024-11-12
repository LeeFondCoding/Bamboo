#include "base/LogStream.h"

#include "gtest/gtest.h"

using namespace bamboo;
using namespace std;


int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

TEST(test_logstream, basic_test) {
    LogStream os;
    os << "hello world";
    os << 1;
    os << 'a';
    bool b = true;
    os << b;
    b = false;
    os << b;
    os << &b;
    std::string a("hello world");
    os << a;
    cout << os.buffer().data() << endl;
}
