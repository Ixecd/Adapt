#define _GLIBCXX_DEBUG
#include <mutex>
#include <thread>

#include "qc.hpp"

/* 对于非 recursive_mutex, 同一个线程对同一个 mutex 重复上锁,
 * 会产生未定义行为(递归死锁) */
/* 最好用std::unique_lock RAII */
int main() {
    std::mutex m1; // err
    std::recursive_mutex m; // ok

    std::thread t1([&] {
        m.lock();
        // ...
        m.lock();       // err
        m.try_lock();   // err, 在另一个线程中没问题,尝试上锁

        m.unlock();
        m.unlock();
    });

    t1.join();

    return 0;
}