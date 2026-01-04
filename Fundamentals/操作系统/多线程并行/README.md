# 基于TBB(Thread Building Blocks)实现并行
- 安装 sudo apt-get install libtbb-dev(ubuntu)
**多核多线程就是为了并行(图形学方向)**
- 一般平时写的cpp文件只会再一个硬件线程上运行,但是可以通过开通多个软件线程再一个硬件线程上交替运行,
- 这样虽然是多个软件线程再运行,实际上只在一个硬件线程上运行代码,实现并发.
- TBB也是创建多个软件线程,但是他会创建刚好数量等于硬件线程(cpu逻辑核)数的软件线程,并设置
- thread affinity(线程对不同cpu核的亲和性)设置后这个线程只能在这个核上跑,也就是说TBB将软件线程
- 和硬件线程绑定了.

**纠正上面的一些混淆**
- 在Linux内核中,线程和进程不区分都是task_struct类
- 线程是操作系统调度的基本单位
- 单进程多线程的场景下是会用到计算机多核计算的能力,因为Linux底层的调度机制是CFS(Completely Fair Scheduler)
- 每一个线程都会被`平等`地调度到CPU上执行,所以其本身就是并行的,只不过一个线程并不会一直在一个核心上运行,上面的TBB其实只是将这个线程绑定到一个指定的核心上
- 这样的话,这个线程就专门跑在这个核心上仅此而已
- 至于为什么使用TBB提速的效果这么好,就是因为其将线程和核心绑定执行,仅此而已

**关于CFS**
- CFS的底层实现是红黑树,其排序是根据virtualrunTime,前面的值越小,表明CPU欠当前进程/线程的时间就越多
- virtualrunTime由很多因素决定,比如输入top指令后有一个**NI**,这个表示`nice`值,nice值越大,表明当前进程/线程越nice,其获取CPU执行的机会就会越小
- nice的值相差10,CPU执行的机会就大概相差10倍

**TBB基于x86架构**
- 使用tbb::task_group 启动多个任务,其含有成员函数run()和wait()
- 一个任务不一定对应一个线程(操作系统层面),如果任务数量超过CPU最大的线程数,会由TBB在用户层负责调度任务运行在多个预先分配好的线程,而不是由操作系统负责调度线程运行在多个物理核心.

- 理想加速比应该是核心数量
- reduce(+)的加速比是逻辑核心数量,for(std::sin)的加速比是物理核心数量
- 原因在于处理的代码是内存密集型还是计算密集型(超线程对内存密集型处理效果比较好)

- 手动计算时间差有点太硬核了,而且只运行一次结果不准确,最好是多次运行取平均值
- 建议使用Google benchmark 
- 只需要将测试代码放在 for (auto _: bm) 即可
- eg.
    void BM_for(benchmark::State &bm) {
        for (auto _: bm) {
            test...
        }
        benchmark::DoNotOptimiz(res); // 让编译器不要对res进行优化,假装res被使用了
    }
    BENCHMARK(BM_for);

    BENCHMARK_MAIN()

- 嵌套tbb::parallel_for出现死锁的原因
    TBB采用了工作窃取法来分配任务,一个线程完成自己队列中的任务之后会从另一个线程的任务队列中取出任务执行,以免自己闲置浪费时间.
    解决方案:使用std::recursive_mutex (解决自己给自己上锁)
            使用tbb::this_task_arena::isolate([&]{}); // 不会互相窃取

- 并行:如何均匀分配任务到每个线程?
    对于并行计算,通常都是CPU有几个核心就开几个线程
    解决1: 线程数量超过CPU核心数量,让系统调度保证各个核心始终饱和.操作系统会自动启用时间片轮转调度,轮流执行每个线程.但是会破坏缓存局域性
    解决2: 线程数量不变使用队列分发任务.这种技术就是线程池,避免了线程需要保存上下文的开销
    解决3: 每一个线程都有一个任务队列,做完本职任务后可以认领其他线程任务
    解决4: 随机分配,通过哈希函数或者线性函数.

**渲染器随机数生成器 Sobol** 
**STD rand 是单线程生成器**

**不连续的vector tbb::concurrent_vector 线程安全,索引访问低效,推荐使用迭代器,不保证高效**
**tbb::concurrent_list/concurrent_set/concurrent_map...**
- 底层实现:分段,1000个元素为一个段,有锁
- 最好的解决办法:先推到一个线程局部(thread_local)的vector中,最后一次性推到concurrent_vector,可以避免锁的竞争
- tbb::concurrent_vector<float> a; auto it = a.grow_by(local_vector.size()); *it++ = local_vector[i++];
- 局部vector reserve一下
- 使用std::copy(from.begin(), from.end(), to.begin(), to.end()) / std::copy(from.begin(), from.end(), std::back_inserter(a));
- 推荐方法:直接对原来的vector预留好足够的空间
- 使用mutex的优化方法:使用tbb::spin_mutex替换std::mutex, spin_mutex不涉及操作系统,spin原子指令,忙等,临界区比较小的情况下.
- 彻底摆脱mutex的方案:通过预先准备好的大小,配合atomic递增索引,同时使用pod模板类,使得vector中resize不会0初始化其中的值
- eg std::vector<pod<float>> a; std::atomic<size_t> a_size = 0;
- GPU上的解决方法:1.算出每个元素需要往vector推送的数据的数量 2.对刚刚算出的数据进行并行扫描,得出每个i要写入的索引
- 3.再次对每个元素并行for循环,根据刚刚生成写入的索引,以此写入数据

**任何递归都可以使用并行优化**
**任务划分的够细时,转为串行,缓解调度负担**

**使用并行+串行(1 << 16)+哈希(mid = std::hash<size_t>{}(size))可以将标准库的qsort提快5倍**
**tbb::parallel_sort**

**简单粗暴并行for**
- 加速比一般只有2/3倍,很不理想.原因:循环体太大,每跑一遍指令缓存和数据缓存都会重新失效一遍,不能很好的利用三级缓存.
- 解决方法:流水线并行6/7倍,对三级缓存比较友好.

**流水线 For CPU**
- 数据之间有依赖
- filter参数:
- serial_in_order 表示当前步骤只允许串行执行,且执行的顺序必须一致
- serial_out_of_order 表示只允许串行执行,但是顺序可以打乱
- parallel 表示可以并行执行当前步骤,且顺序可以打乱
- 每一个步骤filter的输入和返回类型都可以不一样.要求:流水线上一步的返回类型,必须和下一步的输入类型一致.且第一步和最后一步没有返回值为void
- TBB支持嵌套的并行,流水线内部也可以调用tbb::parallel_for进一步并行

- 流水线可以流式处理数据,节省内存512GB -> 4GB(内存)

**核心数量越多,硬件线程越多,CPU计算能力越强,瓶颈转向为内存读取**
**超线程就是为了解决内存卡住的问题产生的**
**为了高效利用缓存,将数据紧凑起来(内存对齐)**
- 缓存行为64B
- 下面这种设计为AOS格式
`struct MyClass {
    float x;
    float y;
    float z;
};`
- 下面这种设计更适合于频繁访问x或y或z的情况(SOA格式)
`struct MyClass {
    float x[n];
    float y[n];
    float z[n];
};`

- AOS(Array of Struct) 单个对象的属性紧挨着存储
- SOA(Struct of Array) 属性分离存储在多个数组
- AOS 必须对齐到2的幂才高效, SOA不需要

**AOSOA 两者兼得**
- 下面的1024并非随意选取,而是要让每个属性SOA数组的大小为一页(4KB),才能最高效
`struct MyClass {
    float x[1024];
    float y[1024];
    float z[1024];
};`
`std::vector<MyClass> mc(n / 1024);`

- good design
`struct PL {
    std::vector<float> pos;
    std::vector<float> vel;
};`
`struct PL {
    float pos_x[1024];
    float pos_y[1024];
    float pos_z[1024];
    float vel_x[1024];
    float vel_y[1024];
    float vel_z[1024];
};`

**预取prefetch**
**预取机制为了保证安全,遇到页边界的时候不再预取**
**缓存和CPU并行操作,CPU执行运算的时候,缓存和内存交涉,读下一个可能的数据**
**建议一开始就分配到页对齐的大小__mm_malloc**
**手动预取_mm_prefetch**

**在只需要写的情况下可以绕开缓存提高性能**
- 为什么写入1比写入0更慢?
- 写入0被编译器自动优化为memset,memset内部使用stream指令绕开缓存不需要读取,直接写入


**内存管理**
- 当调用malloc的时候,操作系统并不会实际分配那一块内存,而是将这一段内存标记为"不可用",当用户试图访问(写入)这一片内存时,硬件就会触发所谓的缺页中断(page fault),进入操作系统内核,内核会查找当前进程的malloc历史记录.如果发现为"可用",下次访问就不会产生缺页中断,如果用户写入的地址根部不是它malloc过的,就会抛出段错误.
- std::vector<int>, new int[n]{} 会初始化数组为0
- malloc(n * sizeof(int)), new int[n] 不会初始化数组为0
- 初始化数组的时候,内存被写入,操作系统才会实际分配内存.
**进一步**
- 操作系统分配内存时按照页面(4KB)来管理的
- malloc一大块,之后访问内存,操作系统并不是把整个区间全部分配完毕,而是只把当前写入地址所在的页面(4KB)给分配上.
- 这就是所谓的操作系统惰性分配特性,也是SPGrid(Sparsely-Paged_Grid)实现的基础.

**标准库中的new和malloc只保证16字节对齐**

**手动池化**
`float func(int n) {
    // 一个线程有一个tmp
    static thread_local std::vector<float> tmp;
    for (size_t i = 0; i < n; ++i) tmp.push_back(i / 15 * 3);
    int ret = tmp[12];
    tmp.clear(); //不会释放内存, 这样下一次再调用这个函数就不需要分配内存
    return ret;
}`
