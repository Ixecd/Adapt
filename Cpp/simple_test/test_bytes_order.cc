#include <stdio.h>

int abs(int x) {
    int mask = x >> 31;  // 如果是负数,符号位会顺下来 1111 1111 -> -1
    printf("mask = %d\n", mask);
    return (mask + x) ^ mask; // (-1 + (-128)) ^ (-1)
    //  1..... 11111111 -> -1
    //  1..... 10000000 -> -128
    // +1....1011111111 -> 
}

int main() {
    // 1,2,3,4  低位----->高位
    // 内存 0x01020304  04 -> 低位, 01 -> 高位, 靠近x的时高位
    // 内存访问是从高地址向低地址
    char buf[4] = {1, 2, 3, 4};
    printf("0x%08x\n", *(int*)buf);  // 小端 -> 4

    int n = 1;
    n = 100;
    int b[n] = {1, 2, 3, 4};
    n = 200;
    int c[n];

    // std::cout << sizeof(b) << std::endl;
    // std::cout << sizeof(c) << std::endl;

    int a = -128;
    printf("%d", abs(a));

    return 0;
}