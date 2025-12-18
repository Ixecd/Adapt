# CUDA 编程
**!虚拟机用不了显卡!**
**可以通过vGPU但是要官网授权,并且主机驱动版本要和虚拟机驱动版本一致,也就是说主机是Windows,虚拟机是Linux也不能用**

**安装CUDA**
- 在CUDA官网选择配置,会给两个终端命令,以此执行即可

__global__ 修饰的函数(void类型)在GPU上运行,从CPU端通过<<<>>>调用,可以有参数,不能有返回值.main在CPU上执行,CPU通过 XXX<<<1,1>>>()调用GPU

__device__ 修饰的函数用于定义设备函数,在GPU上执行,从GPU上调用,不需要<<<>>>

__host__ 函数定义在CPU上

**__host__ 和 __device__ 可以一起用,CPU和GPU都可以调用** 

**constexpr函数自动变成CPU和GPU都可以调用,必须指定--expt-relaxed-constexpr**
- CMake: target_complie_options(main PUBLIC $<$<COMPLIE_LANGUAGE:CUDA>:--expt-relaxed_constexpr>)

- 通过#ifder __CUDA_ARCH__ 判断当前是否处于GPU模式,从而实现一个函数针对GPU和CPU生成两份源码级不同的代码.

__inline__ CUDA编译器的"私货",推荐内联. gcc的"私货"是 __attribute__(("inline))

__forceinline__ CUDA强制内联

__noinline__ CUDA禁止内联

- host可以调用global,global可以调用device,device可以调用device

**CPU和GPU**
- CPU和GPU之间的通信,为了高效,是异步的.
- 同步方法: 调用cudaDeviceSynchronize(),让CPU陷入等待

**<<<first, second>>>**
- first决定板块数量
- second决定启动kernel时所用GPU的线程数量
- 启动 first * second 个线程
- 实际上GPU的板块相当于CPU的线程,GPU的线程相当于CPU的SIMD,但不完全等同