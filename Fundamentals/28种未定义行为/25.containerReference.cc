#define _GLIBCXX_DEBUG
#include <vector>

#include "qc.hpp"

int main() {
    std::vector<int> arr;
    arr.reserve(1);
    arr.push_back(100);

    auto it = arr.begin();
    arr.push_back(200);  // cap 1 -> 2
    PRINT(*it);

    return 0;
}