#define _GLIBCXX_DEBUG
#include <mutex>
#include <thread>

#include "qc.hpp"

/* 多个线程同时对两个mutex上锁,但顺序相反,会产生死锁,未定义行为 */
/* 要保证上锁的顺序每个线程都一样 */
int main() {
    int source = 10;
    std::mutex m1, m2;

    std::thread t1([&] { // err
        m1.lock();
        m2.lock();
        source = 100;
        m2.unlock();
        m1.unlock();
        // 解决办法: std::lock(m1,m2), std::unlock(m1,m2) // 按照地址大小上锁,mutex不可移动
    });

    std::thread t2([&] { // err
        m2.lock();
        m1.lock();
        source = 200;
        m1.unlock();
        m2.unlock();
    });
    t1.join();
    t2.join();
    PRINT(source);

    return 0;
}