#include "base/CurrentThread.h"
#include <iostream>

using namespace bamboo;
using namespace std;

int main() {
    cout << CurrentThread::tid() << endl;
}