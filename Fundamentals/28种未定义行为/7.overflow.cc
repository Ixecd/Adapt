#define _GLIBCXX_DEBUG
#include <iostream>
#include <memory>

// Release -> O3
#include "qc.hpp"
constexpr auto func() {
    int x = INT_MAX;
    // + 1 就修改了符号位 1000... 变成了最小的
    int y =
        (int)((unsigned int)x +
              1);  // 有符号整型溢出是未定义行为,并不保证回环.无符号整型溢出必定会回环,并不是未定义行为.
    // (x + x) / 2 ==> x, 如果x允许回环(x + x) / 2 就不一定等于 x
    return y;
}

constexpr int shift_overflow() {
    return 1 << 32;
}

//[[gnu::noinline]] 告诉gnu不要内联
[[gnu::noinline]] constexpr int shift_overflow_int() { return 1 << 32; }
[[gnu::noinline]] constexpr unsigned int shift_overflow_uint() { return 1 << 32; }

int test_shift_overflow() {
    // 加constexpr会报错(编译器来判断),不加的话会执行返回0,因为编译器不知道用户是否是故意的,int x = shift_overflow_int(); 用户就要0 所以编译器不会报错,但会警告.
    constexpr int x = shift_overflow_int();
    PRINT(x);
    constexpr int y = shift_overflow_uint();
    PRINT(y);
    return 0;
}

/* 建议:如果要用位运算使用无符号的 */
/*     如果在数学上用就使用有符号的 */
int main() {
    // constexpr auto _ = func();
    //(float)_;
    // std::cout << _ << std::endl;
    TICK(int_for);
    // int在x86中会回环,但编译器不这么认为,不开DEBUG,编译器(优化)认为是死循环直接转换为while(1); O1 还是会执行(根据实际的来,不会while(1)), O2 会显示未定义行为, O3 直接while(1)
    for (int i = 1; i != 0;++i);  // 1 -> INT_MAX -> 100... -> 直到 1111...-> 000... 退出
    // 这是在编译器没有任何优化的情况下才会这么执行,如果有优化直接就while(1)
    TOCK(int_for);

    TICK(uint_for);
    // uint因为会回环,编译器就会考虑到这一点
    for (unsigned int i = 1; i != 0; ++i);  // 开了O2 -> 0.00013ms vocal! O1 -> 1300ms 不开 -> 2500ms
    TOCK(uint_for);

    constexpr int x = shift_overflow();
    PRINT(x);

    return 0;
}