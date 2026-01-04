#define _GLIBCXX_DEBUG
#include <vector>
#include <deque>
#include "qc.hpp"

int main() {
    std::vector<int> arr = {1, 2, 3};
    auto it = arr.begin();
    arr.push_back(4);
    *it = 0; // err! cap : 3 -> 6

    /* 如果不需要连续内存,可以改用分段内存的deque容器,其可以保证元素不被移动,迭代器不会失效 */
    std::deque<int> deq{1,2,3,4,5,6,7};
    auto it = deq.begin();
    deq.push_back(100);
    *it = 10; // ok

    return 0;
}

/**
 * CMAKE 
 * `-DCMAKE_BUILD_TYPE=Debug` CMake Debug: -O0 -g; CMake Release: -O3 -DNDEBUG
 * `--config Debug` MSVC Debug: xxxd.dll; MSVC Release: xxx.dll
 * 
 */