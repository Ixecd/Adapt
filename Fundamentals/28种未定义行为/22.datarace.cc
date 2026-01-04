#define _GLIBCXX_DEBUG
#include <thread>
#include <string>
#include "qc.hpp"


/* 多个线程访问同一个对象,其中至少有一个线程是写访问,是未定义行为(数据竞争) */
int main() {
    std::string s;

    // 下面两个线程发生了数据竞争,未定义行为,可以所有线程都读一个数据,如果有线程写就会发生数据竞争
    // 解决办法: 锁
    std::thread t1([&]() {
        s += "hello";
    });

    std::thread t2([&](){
        PRINT(s.size());
    });

    return 0;
}