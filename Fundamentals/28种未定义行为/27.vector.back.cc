#define _GLIBCXX_DEBUG
#include <vector>
#include "qc.hpp"

int main() {

    std::vector<int> arr{};
    int i = arr.back(); // err back()并不会对v是否有最后一个元素做检查,此处相当于解引用了越界的指针
    // back() { return *(end() - 1); } 如果当前vector为空就有问题, check
    int i = arr.empty() ? 0 : arr.back();

    return 0;
}