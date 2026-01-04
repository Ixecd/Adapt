#include <bits/stdc++.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_scan.h>


#include <chrono>

using namespace std;

int test() {
    using double_ms = std::chrono::duration<double, std::milli>;
    auto start = std::chrono::steady_clock::now();

    size_t n = 1 << 26;
    std::vector<float> a(n);
    float res = 0;
    for (size_t i = 0; i < n; ++i) {
        res += std::sin(i);
        a[i] = res;
    }
    std::cout << a[n / 2] << std::endl;
    std::cout << res << std::endl;

    auto finish = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<double_ms>(finish - start).count()
              << " ms" << std::endl;
    return 0;
}

// 并行扫描
// 1. 使用n个线程,每个线程处理2个元素
// 2. 使用1个线程,处理n - 1个元素的的缩并
// 3. 使用n - 1个线程,每个处理2个元素的缩并
int test_t() {
    using double_ms = std::chrono::duration<double, std::milli>;
    auto start = std::chrono::steady_clock::now();

    size_t n = 1 << 26;
    vector<float> a(n);
    // is_final不一定是bool类型
    float res = tbb::parallel_scan(
        tbb::blocked_range<size_t>(0, n), (float)0,
        [&](tbb::blocked_range<size_t> r, float local_res, auto is_final) -> float {
            for (size_t i = r.begin(); i < r.end(); i++) {
                local_res += std::sin(i);
                if (is_final) a[i] = local_res;
            }
            return local_res;
        },
        [](float x, float y) { return x + y; });

    auto finish = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<double_ms>(finish - start).count()
              << " ms" << std::endl;
    return 0;
}

#define TICK(x) auto bench_##x = std::chrono::steady_clock::now();
#define TOCK(x)                                                    \
    std::cout << #x": "                                           \
              << std::chrono::duration_cast<                       \
                     std::chrono::duration<double, std::milli>>(   \
                     std::chrono::steady_clock::now() - bench_##x) \
                     .count()                                      \
              << "ms" << std::endl;

int main() {
    size_t n = 1 << 26;
    TICK(n);

    for (size_t i = 0; i < n; ++i);

    TOCK(n);

    return 0;
}