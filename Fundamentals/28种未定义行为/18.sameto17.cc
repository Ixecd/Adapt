// 这里简单提一下 std::move
#include <iostream>
#include <string>

#include "qc.hpp"
std::string func() {
    std::string s = "hello";
    std::string s2 = std::move(s);
    return s;  // ok 和编译器有关, gcc 会清空s, MSVC 中如果小于15个字节,存到string体内的一个union中,认为拷贝和移动一样, 会返回s
    // 语言认为ok, 标准库作者:不一定是空
}

int main() {
    std::string s = func();
    PRINT(s);

    return 0;
}