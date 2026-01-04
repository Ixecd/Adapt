#define _GLIBCXX_DEBUG
#include <iostream>
#include <memory>

/* reinterpret_cast 后面以不兼容的类型访问 */

int main() {
    int i = 10;
    float f = *(float *)&i; // 浮点在计算机的存储是 1 + 8 + 23 --> 1(符号位), 8(指数位 + 127), 23(底数位), int类型是4个字节存储, 一般机器都是小端存储,低位存在低地址处

    *(int*)(uintptr_t)&i; // ok 取地址强制转换为uintptr_t类型之后又强转为int*,再解引用和原来类型一样

    std::cout << f << std::endl;

    return 0;
}