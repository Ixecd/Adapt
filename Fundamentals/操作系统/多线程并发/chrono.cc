#include <bits/stdc++.h>

#include <chrono>
#include <thread>
using namespace std;

int main() {
    auto t0 = std::chrono::steady_clock::now();
    for (volatile int i = 0; i < 1000000000; i++);
    auto t1 = std::chrono::steady_clock::now();
    auto dt = t1 - t0;
    /**
     * @brief duration_cast可以在任意的duration类型之间转化
     * duration<T,R>表示用T类型表示,并且时间单位是R
     *        R省略不写就是秒,std::milli表示毫秒,std::micro表示微秒
     *        using seconds = std::chrono::duration<int64_t>;
     *        using milliseconds = std::chrono::duration<int64_t, std::milli>;
     *        using double_ms = std::chrono::duration<double, std::milli>;
     */
    int64_t ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(dt).count();
    // 更高的精度
    using double_ms = std::chrono::duration<double, std::milli>;
    double dms = std::chrono::duration_cast<double_ms>(dt).count();
    cout << "time elapsed: " << ms << " ms" << endl;
    cout << "time elapsed: " << dms << " dms" << endl;

    // 当前线程睡眠,这个是睡一段时间
    std::this_thread::sleep_for(std::chrono::milliseconds(144));
    // 睡到一个时间点
    auto t = std::chrono::steady_clock::now() + std::chrono::milliseconds(400);
    std::this_thread::sleep_until(t);
    return 0;
}