#include <bits/stdc++.h>

using namespace std;

#include <iostream>
#include <vector>
#include <memory>


/*
    [[no_unique_address]] 是C++20引入的属性,意在告诉编译器可以将这个数据成员和其他成员
    共享内存地址,以便更好的优化内存布局(大多数标准分配器在某些实现中可能是空的)
    与union不同,其用于空类型成员,避免内存浪费.
    union是一种数据结构,是允许多个成员共享同一块内存,但是成员之间是互斥的,任何时刻只有一个成员有效
*/
namespace qc {
    using vector = std::vector<int, std::allocator<int>>;
}

struct S {
    qc::vector v;
    std::allocator<int> a;

    S() : v(a) {} // 用同一个分配器初始化vector
};

int main() {
    S s;
    s.v.push_back(1);
    s.v.push_back(2);

    // 打印分配器的地址
    std::cout << "Address of allocator in struct S: " << &s.a << std::endl;
   // std::cout << "Address of allocator in vector: " << &(s.v.get_allocator()) << std::endl;

    return 0;
}
