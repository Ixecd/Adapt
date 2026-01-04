#include <bits/stdc++.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>
using namespace std;

// 并行缩进 O(logn) 针对GPU
// 可以避免浮点误差,eg求平均值
// 浮点数不能大加小 10000000000000.0 + 0.000000000001 = 10000000000000.0

int main() {
    using double_ms = std::chrono::duration<double, std::milli>;
    auto start = std::chrono::steady_clock::now();

    size_t n = 1 << 26;
    // tbb::parallel_deterministic_reduce -> 保证每次运行结果一致.
    float res = tbb::parallel_reduce(
        tbb::blocked_range<size_t>(0, n), (float)0,
        [&](tbb::blocked_range<size_t> r, float local_res) -> float {
            for (size_t i = r.begin(); i < r.end(); ++i)
                local_res += std::sin(i);
            return local_res;
        },
        [](float x, float y) { return x + y; });

    std::cout << res << std::endl;

    auto finish = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<double_ms>(finish - start).count()
              << " ms" << std::endl;
    return 0;
}