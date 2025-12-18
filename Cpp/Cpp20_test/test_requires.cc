#include <bits/stdc++.h>
using ll = long long;
using namespace std;

template <class T>
void tru_call_foo(T &t) {
    // requires 语法, 检测一个表达式否能合法通过编译
    // 如果能, reuqires 会返回true
    // if constexpr(Cpp17) 进行编译器分支判断
    if constexpr (requires { t.foo(); }) {
        t.foo();
    }
}

int main() { return 0; }