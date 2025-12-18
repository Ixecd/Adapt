#include <bits/stdc++.h>

#include "qc.hpp"
using namespace std;
// tuple中不允许使用void类型,所以要自己写一个Uninitialized类表示void
template <class T>
struct Uninitialized {
    union {
        T val;
    };
    // 移出去
    T moveValue() {
        T ret = std::move(val);
        return ret;
    }
    template <class... Ts>
    void putValue(Ts &&...ts) {
        std::construct_at(&val, std::forward<Ts>(ts)...);
    }
};
// 显示特化
template <>
struct Uninitialized<void> {
    void moveValue() { PRINT_S(Typevoid); }
};
// 泛型模板,如果用户没有指定则为void, 和第一个struct是冲突的.
// 这种没有T
template <class = void>
struct Uninitialized {
    void moveValue() { PRINT_S(Typevoid); }
};

int main() {
    tuple<Uninitialized<int>, Uninitialized<void>> tup;
    std::get<0>(tup).putValue(100);
    PRINT(std::get<0>(tup).moveValue());

    return 0;
}