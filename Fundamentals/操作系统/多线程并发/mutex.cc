#include <bits/stdc++.h>
using namespace std;

int test_mutex() {
    std::vector<int> arr;
    std::mutex mtx;
    std::thread t1([&] {
        for (int i = 0; i < 1000; ++i) {
            mtx.lock();
            arr.push_back(1);
            mtx.unlock();
        }
    });

    std::thread t2([&] {
        for (int i = 0; i < 1000; ++i) {
            mtx.lock();
            arr.push_back(2);
            mtx.unlock();
        }
    });

    t1.join();
    t2.join();

    cout << arr.size() << endl;
    return 0;
}

/**
 * @brief std::lock_guard 符合RAII思想的上锁和解锁
 *          将锁的持有视为资源,上锁视为锁的获取,解锁视为锁的释放
 *          std::lock_guard 其构造函数里就会嗲用mtx.lock()
 *          解构函数就会调用mtx.unlock();
 *          不能动态unlock.
 *        std::unique_lock 可以动态去unlock().
 *          C++11 : std::unique_lock<std::mutex> ulk(mtx);
 *          C++17 : std::unique_lock ulk(mtx);
 */

int test_lock_guard() {
    std::vector<int> arr;
    std::mutex mtx;
    std::thread t1([&] {
        for (int i = 0; i < 1000; ++i) {
            std::lock_guard grd(mtx);
            arr.push_back(1);
            // grd.unlock(); // ERROR
        }
    });

    std::thread t2([&] {
        for (int i = 0; i < 1000; ++i) {
            std::lock_guard grd(mtx);
            arr.push_back(2);
        }
    });

    t1.join();
    t2.join();

    cout << arr.size() << endl;
    return 0;

    return 0;
}
/**
 * @brief
 * 只要具有某些指定名字的成员函数,就判断一个类是否满足某些功能的思想,在Python中称为鸭子类型
 *        而在C++中称为concept(概念).比起虚函数和动态多态的接口抽象,concept使得实现和接口更加解耦
 *        且没有性能损失.
 *
 */

int test_unique_lock() {
    std::vector<int> arr;
    std::mutex mtx;
    std::thread t1([&] {
        for (int i = 0; i < 1000; ++i) {
            std::unique_lock ulk(mtx);
            arr.push_back(1);
        }
    });

    std::thread t2([&] {
        for (int i = 0; i < 1000; ++i) {
            // 指定了第二个参数的话,std::unqiue_lock不会在构造函数中调用mtx.lock(),需要之后再手动调用grd.lock()才能上锁.
            // std::defer_lock 表示延迟上锁
            // ---------- defer_lock 作为参数
            std::unique_lock ulk(mtx, std::defer_lock);
            // ---------- try_to_lock 作为参数
            // std::unique_lock ulk2(mtx, std::try_to_lock);
            // if (!ulk2.owns_lock())
            //    ulk.lock();
            // else
            //    cout << "get lock" << endl;
            // ---------- adopt_lock 作为参数
            // 之前已经上锁了,但是之后仍然希望用RAII思想在解构时候自动调用unlock(),可以用std::adopt_lock作为第二个参数
            // ulk.lock();
            // std::unique_lock ulk2(mtx, std::adopt_lock);
            // ulk.unlock();
            ulk.lock();
            arr.push_back(2);
            ulk.unlock();
            ulk.lock();
        }
    });

    t1.join();
    t2.join();

    cout << arr.size() << endl;

    return 0;
}

/**
 * @brief try_lock() 如果上锁失败,不要等待,返回bool
 *        try_lock_for(std::chrono::milliseconds(400))
 * 如果上锁了等待400ms,如果还没释放锁就继续
 *        try_lock_until(std::chrono::steady_lock::now() +
 * std::chrono::milliseconds(500));
 */

std::mutex mtx1;
int test_try_lock() {
    if (mtx1.try_lock())
        cout << "succ" << endl;
    else
        cout << "fail" << endl;

    if (mtx1.try_lock())
        cout << "succ" << endl;
    else
        cout << "fail" << endl;

    test_unique_lock();

    return 0;
}

/**
 * @brief 使用std::lock 同时对多把锁上锁
 *        std::lock(mtx1,
 * mtx2),其接受任意多个mutex作为参数,并且他保证在无论任意线程中嗲用的顺序是否相同,都不会产生死锁问题.
 *        std::lock 和 std::unlock不是RAII
 */

/**
 * @brief 为了解决上面不满足RAII,使用std::scoped_lock grd(mtx1, mtx2)
 */

/**
 * @brief 单线程中死锁情况, std::recursive_mutex mtx1;
 */

/**
 * @brief 读写锁 std::shared_mutex smtx;
 *       smtx.lock_shared(); 上读锁
 *       smtx.lock();        上写锁
 * RAII
 * unique_lock 针对 lock(); --> std::unique_lock grd(smtx);
 * shared_lock 针对 lock_shared(); --> std::shared_lock grd(smtx);
 */

// 条件变量, 必须和 mutex一起用
// std::condition_variable 仅仅支持 std::unique_lock<std::mutex> 作为wait的参数,
// 如果需要用其他类型的mutex锁,可以用std::condition_variable_any
// wait_for, wait_until
// 也可以等待某一条件成真
int test_condition() {
    std::condition_variable cv;
    std::mutex mtx;
    bool ready = false;
    
    std::thread t1([&] {
        std::unique_lock lock(mtx);
        // cv.wait(lock);
        cv.wait(lock, [&] { return ready; });
        lock.unlock();
        std::cout << "t1 is awake" << endl;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    cout << "notifying.." << endl;
    cv.notify_one();

    ready = true;
    cv.notify_one();
    t1.join();

    return 0;
}

int main() {
    // 原子操作,专门硬件指令加持 += -= ++ -- &= |= ^=
    std::atomic<int> count {0};
    std::atomic<int> counter;
    counter.store(0);

    count += 1; // ok 等价于 counter.fetch_add(1);
    // 注意fetch_add可以返回右值 int old = counter.fetch_add(1);
    // int old = couner.exchange(val)
    count = count + 1; // no 不能保证原子性

    cout << counter.load() << endl; // 加载数据

    return 0;
}