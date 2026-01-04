#include <bits/stdc++.h>
#include <tbb/parallel_pipeline.h>
#include <tbb/parallel_for.h>
#include <chrono>
using namespace std;

struct Data {
    void step1() {  val++; }
    void step2() {  val += 2; }
    void step3() {  val += 3; }
    void step4() {  val += 4; }
    int val = 0;
};

int main() {
   using double_ms = std::chrono::duration<double, std::milli>;
   auto start = std::chrono::steady_clock::now();

    //任务之间有先后顺序
    size_t n = 1 << 11;
    std::vector<Data> datas(n);
    auto it = datas.begin();
    //创建流水线, 最多有8个线程,8个Data在流水线上跑
    tbb::parallel_pipeline(8, tbb::make_filter<void, Data*>(tbb::filter_mode::serial_in_order, [&] (tbb::flow_control &fc) -> Data* {
        if (it == datas.end()) {
            fc.stop();
            return nullptr;
        }
        // 必须是串行的,有数据依赖
        return &*it++;
    })
    , tbb::make_filter<Data*, Data*>(tbb::filter_mode::parallel, [&](Data *dat) -> Data * {
        dat->step1();
        return dat;
    })
    , tbb::make_filter<Data*, Data*>(tbb::filter_mode::parallel, [&](Data *dat) -> Data * {
        dat->step2();
        return dat;
    })
    , tbb::make_filter<Data*, Data*>(tbb::filter_mode::parallel, [&](Data *dat) -> Data * {
        dat->step3();
        return dat;
    })
    , tbb::make_filter<Data*, void> (tbb::filter_mode::serial_out_of_order, [&](Data *dat) -> void {
        dat->step4();
    })
    );

   auto finish = std::chrono::steady_clock::now();
   std::cout << std::chrono::duration_cast<double_ms>(finish - start).count() << " ms" << std::endl; // 6ms
    return 0;
}