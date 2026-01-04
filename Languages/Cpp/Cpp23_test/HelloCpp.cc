// 非常可惜,目前只有MSVC 和 Clang实现了std::print
// #include <print>
#include <bits/stdc++.h>

int main() {
    // Each call to std::println ends with new-line
    std::println("Please"); // overload (1)
    std::println("enter"); // (1)

    std::print("pass");
    // GCC 已经提出来的,但是没有实现
    // __cpp_lib_print("pass");
    // __cpp_lib_format("word");
    std::print("word");

    std::println(); // (3); valid since C++26; same effect as
    std::print("\n");
    return 0;
}