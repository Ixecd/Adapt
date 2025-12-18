#include <bits/stdc++.h>
using namespace std;

int main() {
    vector v = {4, 3, 2, 1, 0, -1, -2};

    // 先把vi送到filter中处理,再送到transform中处理,类似于linux中的管道
    for (auto &&vi : v | views::filter([](auto &&x) { return x >= 0; }) | views::transform([](auto &&x) { return sqrtf(x); })) {
        cout << vi << endl;
    }

    return 0;
}