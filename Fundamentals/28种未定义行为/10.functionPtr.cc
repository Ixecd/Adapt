#define _GLIBCXX_DEBUG

#include "qc.hpp"

void func() { PRINT_S(func called); }

typedef void (*func_t)();
static func_t fp = nullptr;

void set_fp() { fp = func; }

/* 函数指针被调用时, 不能为空 */
int main() {
    (*func)();  // error!
    // clang 会认为其他文件中调用了set_fp()之后才会fp(),clang 认为函数指针永远不会为空. gcc 会抛出SIGSEVG,但也会输出出来
    // 和优化有关系.
    fp();

    return 0;
}