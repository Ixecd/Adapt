# Anim 八 Pass 交织管线

## 从源码到信号绳

```
.anim 源码
    ↓ Pass 0: LexParse       词法 → Token → AST
    ↓ Pass 1: TypeCheck      感受类型校验 + 设备前置告警
    ↓ Pass 2: StaticSafety   源码级安全规则（不依赖用户上下文）
    ↓ Pass 3: UserStateSafety  源码 × 用户上下文（创伤分型 × cap × 未成年）
    ↓ Pass 4: RuntimeGuard   运行期安全插桩生成（交织期预埋）
    ↓ Pass 5: FSIRGen        感受结构 IR
    ↓ Pass 6: Personalize    FSIR × PBM → PSIR（个人基线左乘）
    ↓ Pass 7: DeviceMap      PSIR → DSIR（设备分配 + 算力感知交织）
    ↓ Pass 8: CodeGen        DSIR → ESIR（帧级指令，每 1ms）
    ↓
ESIR 帧 -> FPGA -> 设备 -> 神经通路 -> 感受
```

## 为什么是八 Pass

每个 Pass 只做一件事——织入一股新信息流或施加一层新安全约束。

```
Pass 0    织入源码结构。AST 是后续全部 Pass 的输入。
Pass 1    织入感受类型信息。查 Pattern Registry。设备前置告警——不是报错，是标记降级。
Pass 2    施加第一层安全。不需要用户上下文。源码自己就可能不安全。
Pass 3    施加第二层安全。同一份源码——不同用户不同结果。
Pass 4    预埋。不拒绝——加栅栏。运行时不需要判断——栏杆已经在那。
Pass 5    织入感受语义。FSIR 保留 origin_shape/origin_intensity 语义锚点——给逆向调试用的。
Pass 6    织入个人基线。PBM 永不离设备——这步在设备本地完成。
Pass 7    织入设备约束。算力感知——入门设备仅耳后 vs 高端全套。
Pass 8    织入实时反馈。自适应帧密度。闭环预修正。
```

## Pass 边界——不冲刷 CPU 流水线

```
传统编译器：每个 Pass 输出到文件 -> 下一个 Pass 读文件 -> 序列化开销。
Anim：Pass 之间的 IR 传递 = 指针传递。不序列化。不写磁盘。
CPU 指令流水线不被 Pass 边界打断。
八个 Pass 在 CPU 上是八个函数调用——不是八个独立进程。
```

## 为什么不是 4 或 12 个

```
4  Pass: 太粗。安全层混在一起 -> 离线缓存命中率低。
12 Pass: 太细。每次序列化开销累积 -> 延迟超标。
8  Pass: Pass 2/3 不能合并（一个不要用户上下文、一个必须读用户档案）。
         Pass 6 不能和 5 合并（PBM 永不离设备）。
```
