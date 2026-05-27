# Anim 四层 IR

## 不是抽象层次——是独立流

传统编译器 IR 从高抽象到低抽象（HIR → MIR → LIR）。Anim 的 IR 每层织入一股不属于前面任何一股的外部变量。层数由独立流的股数决定。

```
FSIR    Feeling Structure IR       感受结构——主旋律、点缀、形状、强度
                                   与人无关。与设备无关。纯粹的感受语义声明。

PSIR    Personal Signal IR         个人适配——FSIR × PBM（个人基线左乘）
                                   此人的平静 = 迷走神经 0.38mA（不是通用 0.5mA）

DSIR    Device Signal IR           设备分配——信号参数分解为多设备协同指令
                                   耳后 0.4mA + 后颈 3kPa + 腕部 36.5°C

ESIR    Execution Signal IR        帧级执行——每 1ms 一帧，带闭环反馈指针
                                   电流/频率/脉宽/温度/振动/时间戳（微秒精度）
```

## 为什么是四层

```
三层（砍 DSIR）：
    设备分配塞进 PSIR 或 ESIR。
    塞 PSIR → 设备一变 PSIR 重算 → PBM 跟着跑 → 隐私/性能双炸。
    塞 ESIR → FPGA 每一帧跑设备映射 → 延迟超标。

五层（加 TSIR 时序调度层）：
    shape 的时刻已经在 Pass 8 自适应帧密度里展开。
    加一层 = 多一次序列化/反序列化 = 每帧延迟不是零。
    热路径不加层。
```

## 层间隐私

```
FSIR 无个人数据 → 可缓存至 Server。
PSIR 含 PBM → 永不离设备。FSIR→PSIR 在设备本地完成。
    不是性能优化——是隐私架构的硬约束。
DSIR/ESIR 含设备生理参数 → Session 期间只在设备内存和 FPGA 之间跑。
```

## 每层的内容边界

| | FSIR | PSIR | DSIR | ESIR |
|---|---|---|---|---|
| 感受语义 | ✓ | - | - | - |
| 个人基线 | - | ✓ | - | - |
| 设备分配 | - | - | ✓ | - |
| 帧级反馈 | - | - | - | ✓ |
| 缓存位置 | Server | Device | Device RAM | FPGA reg |
