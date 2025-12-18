// import <vector>;
// import <iostream>;
// import <numeric>;
// import <ranges>;
// import <cmath>;

#include <cmath>
#include <iostream>
#include <numeric>
#include <ranges>
#include <vector>

using namespace std;

int main() {
    const vector v = {4, 3, 2, 1, 0, -1, -2};

    // 先把vi送到filter中处理,再送到transform中处理,类似于linux中的管道
    // 右值并不是引用哦,不会改变原来数组里的数据 
    for (auto &&vi : v | views::filter([](auto &&x) { return x >= 0; }) |
                         views::transform([](auto &&x) { return sqrtf(x); })) {
        cout << vi << endl;
    }

    for (int num : v) std::cout << num << " ";

    return 0;
}