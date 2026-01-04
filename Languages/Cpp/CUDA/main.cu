#include <cstdio>

/// @brief __global__ 表示让他在GPU上执行, CPU可以通过 xxx<<<1,1>>>调用GPU上的函数
__global__ void kernel() {
    printf("Hello, CUDA!\n");
}

int main() {
    kernel<<<1,1>>>();
    cudaDeviceSynchronize();
    return 0;
}