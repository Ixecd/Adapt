#define _GLIBCXX_DEBUG
#include <cstring>
#include <iostream>
#include <memory>

constexpr float func() {
    // union 如果访问的不是上一次激活的属性就是未定义行为 -> 访问(读取,返回)
    // 激活(写入)
    union {
        int i;
        float f;
    } u;
    u.i = 100;
    // float f = u.f;
    // std::cout << "u.f = " << f << std::endl;
    return u.f;  // error!
    // 由于是同一块内存,运行的时候并不会报错,编译器会默认将int类型转化为float类型返回,加上constexpr
    // 就会在编译阶段报错
}

void test_memcpy() {
    int i = 100;
    float f;
    // memcpy是一个非编译期的函数,所以不会检测出来问题.
    std::memcpy(&f, &i, sizeof(i));
    std::cout << "test_memcpy(): f = " << f << std::endl;
}

// 解决方案:C++20 std::bit_cast<T>
constexpr int test_bitCast(int i) {
    float f = std::bit_cast<float>(i); // 编译期的函数 -> 也就是加了constexpr修饰
    // cout 是非编译期的函数,所以在constexpr中不能出现
    // std::cout << "f = " << f << std::endl;
    return 1;
}

/* union访问不是激活的对象 */
int main() {
    // constexpr float f = func();
    // test_memcpy();
    constexpr int j = test_bitCast(100); // ok
    return 0;
}