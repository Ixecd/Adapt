#define _GLIBCXX_DEBUG
#include <memory>
#include "qc.hpp"
constexpr int func() {
    int x = 10;
    int y = 0;
    return x / y;
}

int main() {

    // 不是常量表达式,里面有错,有未定义行为
    constexpr auto i = func();
    PRINT(i);

    return 0;
}