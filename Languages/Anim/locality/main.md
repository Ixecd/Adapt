# Anim 局部性——1ms 硬死线

## 物理约束

```
1ms = 1,000,000ns。
一次 L3 cache miss = ~40-100ns。
一次主内存访问 = ~100ns。
一次磁盘 I/O = ~10ms —— 不在这个时钟域。

1ms 内 FPGA 门级逻辑必须完成：
    读 ESIR 帧 → DAC 输出 → ADC 回读 → 偏差计算 → 下一帧修正。

Cache miss 累积到 ~100μs 以上 → 帧超时。Session 掉帧。
Anim 的全部数据布局以局部性为第一原则。
不是优化——是能不能在 1ms 内完成。
```

## 时间局部性——刚碰过的别再从主存拉

```
ESIR 帧缓冲区：原地更新。不是每帧新分配。
    帧 N+1 在帧 N 同一块内存上直接改。
    不变的字段（频率、压力、温度）不写、不读、不移动。
    只有 0.01mA 的增量被更新。
    环形缓冲区 -> FPGA DMA 循环读同一块物理内存 -> L1 永远命中。

PBM 系数：一个 Session 只读一次。
    Pass 6 启动时载入 L2。之后不再碰主存。

安全插桩阈值：一次加载到 FPGA 寄存器。N 帧循环比较。不重新 fetch。
```

## 空间局部性——一起用的一起存

```
ESIR 帧按设备分组连续存储：
    耳后 [current, freq, pulse_width] 12 bytes → 一个 cache line
    后颈 [pressure, freq, pattern]       12 bytes → 一个 cache line
    腕部 [temp, vibration, duration]    12 bytes → 一个 cache line
    FPGA 读一个设备 → 一次 cache line 加载。不全读。不同设备不共享 cache line = 无 false sharing。

FSIR 混音结构连续存储：
    主旋律头（8 bytes）→ 点缀数组长度（2 bytes）→ 点缀条目连续（每条 16 bytes）。
    一次加载 → 主旋律 + 全部点缀在 L1。

Pattern Registry 按共现频率分桶：
    accomplishment_satisfaction + exhaustion_relief + slight_void + self_assurance
    → 同一源码包内共现频率最高 -> 物理存储相邻页面。
    一次磁盘页加载 -> 四个原子全在内存。
```

## 双流水线缓存隔离

```
前台热路径（DSIR → ESIR + FPGA）：
    跑在 CPU 的一个物理核心上。L1/L2 专有。不与后台共享。
    只跑 Pass 7-8。帧缓冲在 FPGA 附近的 SRAM 上。

后台冷路径（Pass 0-5）：
    跑在另一个核心上。自己的 L1/L2。
    离线预交织。不需要实时。FSIR 缓存到本地磁盘。

两条线的 L1/L2 不交叉 = 不会互相污染 cache = 各自 100% 命中。
不是逻辑隔离——是物理 cache line 不交叉。
```
