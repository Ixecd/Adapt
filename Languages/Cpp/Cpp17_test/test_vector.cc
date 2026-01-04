#include <bits/stdc++.h>
using namespace std;

int main() {
    // CTAD compile-time argument deducation/编译器参数推断
    vector v = {1, 2, 3, 4};
    int sum = 0;
    for_each(v.begin(), v.end(), [&](auto vi) { sum += vi; });

    std::cout << sum << std::endl;

    return 0;
}
