#define _GLIBCXX_DEBUG
#include <cstring>
#include "qc.hpp"

/* memcpy 函数的 src 和 dst 不能为空指针 */
int main() {

    void *src = nullptr;
    void *dst = nullptr;
    size_t size = 0;
    memcpy(dst, src, size); // err
    if (size != 0)
        memcpy(dst, src, size); // ok

    return 0;
}