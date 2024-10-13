#pragma once

#include "base/Macro.h"

#include <condition_variable>
#include <mutex>

namespace bamboo {

class CountDownLatch {
public:
    explicit CountDownLatch(int count);

    DISALLOW_COPY(CountDownLatch)

    void wait();

    void countDown();

    int getCount() const;

private:
    mutable std::mutex mutex_;

    std::condition_variable con_var_; //GUARDED_BY(mutex_)

    int count_; // GUARDED_BY(mutex_)
};

}