#define _GLIBCXX_DEBUG
#include "qc.hpp"

constexpr auto func() {
    int arr[10]{0}; // 初始化

    int *p = arr;
    p += 1; //ok
    p += 9; //ok 可以产生指向数组尾部的指针,但是尾部指针不能解引用
    //p += 10; //err 越界

    return arr[0];
}

/* 指针加减法不能越界 */
int main() {

    constexpr auto i = func();

    PRINT(i);


    return 0;
}