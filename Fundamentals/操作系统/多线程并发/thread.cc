#include <bits/stdc++.h>
using namespace std;

/**
 * @brief C++11开始,为多线程提供了语言级别的支持.使用std::thread来表示线程.
 *        std::thread 构造函数的参数可以是任意lambda表达式.
 * @details 作为一个C++类,std::thread 同样遵循RAII思想和三五法则:因为管理着资源
 *          其自定义了解构函数,删除了拷贝构造/赋值函数,提供了移动构造/赋值函数.
 */

auto threadfunc() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "threadFunc() end" << std::endl;
}

auto mufunc() {
    std::thread t1([&] { threadfunc(); });
    // 退出函数的时候,会销毁t1线程句柄
    // 分离和杀死是两个概念
    // 解决办法:
    // 把t1对象移动到一个全局变量中,从而延长其生命周期到myfunc函数体外.(不推荐)
    //  创建线程池,在线程池解构的时候把所有线程都join一遍.(推荐)
    t1.detach();  // 并不会自动join, join会阻塞等待,detach分离,如果主线程退出它也就没了
    // std::thread符合RAII原则,myfunc退出之后就对调用t1的解构函数(如果不detach的话)
}

/**
 * @brief C++20中引入了std::jthread, 符合RAII思想,解构时自动join

 */

int main() {
    mufunc();
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    return 0;
}