#include <cstring>
#include "qc.hpp"

int main() {

    char arr[10]{};
    // 参数有 __restrict__ 不允许有任何重叠
    memcpy(arr, arr + 1, 9); //err memcpy
    memcpy(arr + 1, arr, 9); //err
    // 解决办法: 参数没有__restruct__ 可以有重叠
    // memmove();

    return 0;
}