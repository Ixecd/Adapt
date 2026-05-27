# Anim 的丝与交叉——Cross Over

## Anim 的数据不是值——是丝

```
Rust:
    值在堆上。值在栈上。值有 owner。owner 可以借出去。
    &T → 借用。*T → 解引用。0x7ffe1234 → 地址。
    这是 RAM 模型。

Anim:
    数据在织机上。一根丝 = 一股独立的信号流。
    丝从纺锤（Spindle）出——纺锤 = 产生它的源。

    四根主要经线（Warp）：
        FSIR 的纺锤 = .anim 源码
        PSIR 的纺锤 = 个人基线矩阵 (PBM)
        DSIR 的纺锤 = 设备集合
        ESIR 的纺锤 = 实时回读环
```

## 一等操作：cross_over(warp, weft)

```
Rust 的一等操作：x = &y;    ← 借用
Anim 的一等操作：warp.cross_over(&weft)

cross_over 不是借用。
cross_over = 两根丝在同一个时空坐标相遇。
交叉点不是一根丝引用另一根丝——是两根丝共同产生一个值。

warp:  "此人的平静 = 迷走神经 0.4mA"          ← FSIR × PBM 产生
weft:  "上一帧心率偏了 +2bpm，需要 +0.01mA"    ← 闭环偏差
cross: 0.41mA。                                ← 共同输出。不是借用。

交叉之后：
    warp 继续向前。等下一根 weft 来交叉。
    weft 退场。下一帧是新 weft。
    两根丝方向不变。所有权不变。
```

## 织入（Weave）

一根 weft 穿过整排 warp → 一路产生交叉点 → 交叉点连成线 → 织入完成。

```
Session 启动——
    weft = 初始信号。
    穿过 -> FSIR, PBM, DeviceSet
    -> 每个交叉点输出一段参数 -> 连成一条完整的 ESIR 帧。

Session 运行中——
    每 1ms 一根新 weft（偏差修正量）。
    穿过同一排 warp。一路新交叉 -> 新 ESIR 帧。
```

## 回退（Uncross）

```
uncross(warp, weft, at)

安全停止帧触发 -> 当前帧的交叉点被撤销。
这帧交叉没发生。下一帧继续。
不是 panic。不是 drop。只是这帧不走。
warp 还在。下一帧照常。
```

## 绳结（Knot）

```
Rust 的"存哪" = 0x7ffe1234（内存地址）
Anim 的"在哪" = 这根丝上次与谁交叉过

PBM 和 FSIR 在 Pass 6 交叉过 47 次。
每次交叉产生的交叉点 = 一个绳结。
第 48 次 session 的 weft 来交叉时——
    先找最近的绳结。
    不用从零开始。
    绳结给出起始偏差。快速收敛。
```

## Cross Checker——替代 borrow checker

```
Rust borrow checker:
    &mut 不能同时存在两处。防止 use-after-free。

Anim cross checker:
    一根不能和创伤原子交叉的 warp——永远没有被创伤 weft cross over 的点。
    一根丝不能和强度超过 cap 的 weft 交叉。
    不是内存冲突——是线冲突。
    不是"借了别人的"——是"没有交叉资格"。
```
