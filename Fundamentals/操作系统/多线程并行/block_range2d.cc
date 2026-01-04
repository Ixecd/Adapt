#include <iostream>
#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>
#include <vector>
#include <cmath>

int main() {

    size_t n = 1 << 13;
    std::vector<float> a(n * n);

    // 行列
    tbb::parallel_for(tbb::blocked_range2d<size_t>(0, n, 0, n), [&] (tbb::blocked_range2d<size_t> r) {
        // col是列,表示行的范围
        for (size_t i = r.cols().begin(); i < r.cols().end(); ++i) {
            for (size_t j = r.rows().begin(); j < r.rows().end(); ++j) {
                a[i * n + j] = std::sin(i) * std::sin(j);
            }
        }
    });

    // 同理有三维的 tbb/blocked_rang3d.h

    return 0;
}