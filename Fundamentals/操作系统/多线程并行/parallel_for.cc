#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

#define TICK(x) auto bench_##x = std::chrono::steady_clock::now();
#define TOCK(x)                                                    \
    std::cout << #x " : "                                          \
              << std::chrono::duration_cast<                       \
                     std::chrono::duration<double, std::milli>>(   \
                     std::chrono::steady_clock::now() - bench_##x) \
                     .count()                                      \
              << " ms" << std::endl;
/// @brief tbb::parallel_for可以再给一个参数 tbb::static_partitioner{} 分区器
/// 平均分配(木桶原理,效果不好),
///        tbb::simple_partitioner{} 将任务轮换的分配给线程(不会受限于木桶原理
///        粒度太细 效果也不好)
///         tbb::simple_partitioner
///         能够按照给定的粒度将矩阵分块,块内部小区域按照常规的两层循环访问,以便矢量化,
///         块外部大区域以类似Z字形便利,利用局部性原理,更高效,缓存友好
///         上面两种都可以指定粒度tbb::blocked_range<size_t>(0, n, 6) -> 粒度为6
///         根据粒度可以进行优化 tbb::auto_partitioner{} 自动分区智能
///         tbb::affinity_partitioner
///         记录历史,下次根据经验自动负载均衡.必须提前定义.
int main() {
    size_t n = 1 << 26;
    std::vector<float> a(n);

    auto start = std::chrono::steady_clock::now();
    using double_ms = std::chrono::duration<double, std::milli>;
    //    TICK(parafor);
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, n),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); ++i) a[i] = std::sin(i);
        },
        tbb::static_partitioner{});

    // 面向初学者的接口(不能优化)
    tbb::parallel_for((size_t)0, (size_t)n,
                      [&](size_t i) { a[i] = std::sin(i + 1); });

    // 基于迭代器区间:parallel_for_each
    tbb::parallel_for_each(a.begin(), a.end(), [&](float &f) { f = 32.f; });

    //    TOCK(parafor);
    auto finish = std::chrono::steady_clock::now();
    std::cout << "spend time : "
              << std::chrono::duration_cast<double_ms>(finish - start).count()
              << " ms" << std::endl;

    std::cout << "single thread:" << std::endl;
    start = std::chrono::steady_clock::now();
    for (size_t i = 0; i < a.size(); ++i) a[i] = std::cos(i);
    finish = std::chrono::steady_clock::now();
    std::cout << "thread sepnd : "
              << std::chrono::duration_cast<double_ms>(finish - start).count()
              << " ms" << std::endl;

    return 0;
}