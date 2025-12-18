// C++20 struct直接可以通过()初始化, 生成一个默认构造函数
#include <bits/stdc++.h>
using namespace std;

int main() {
    struct S {
        int x;
        int y;
    };

    S s(1, 5);

    std::cout << s.x << " " << s.y << std::endl;

    return 0;
}