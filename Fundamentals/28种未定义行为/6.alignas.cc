#define _GLIBCXX_DEBUG
#include <iostream>
#include <memory>

#include "qc.hpp"

/* T类型指针必须对齐到 alignof(T) */

struct alignas(64) C {
    int i;
    char c;
};

int main() {
    C *p = (C*) malloc(sizeof(C)); // err malloc产生的指针只保证对齐到max_align_t(GCC 上是 16字节), 并不保证对齐到C所需的64字节

    C *p = new C; // ok new T 总是保证对齐到alignas(T)

    char buf[sizeof(int)];
    int *p = (int *)buf; // err! buf 不一定对齐到4字节,只是大小刚好是4

    // 解决办法 1
    // alignas(sizeof(int)) ok 将 buffer对齐到4字节
    alignas(alignof(int)) char buffer[sizeof(int)];
    int *q = (int *)buffer; // ok

    // 解决办法 2
    char buff[sizeof(int) * 2];
    int *t = (int *)(((uintptr_t)buff + sizeof(int) - 1) & ~(alignof(int) - 1)); // ok 对齐到int

}