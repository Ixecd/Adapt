# Anim 错误处理——oi

## 不是 Err。是 oi。

```
Rust   Result<T, E> + ?    错误是操作失败。堆栈可追溯。可恢复。
Go     if err != nil        错误是返回值。显式检查。调用方决定。
C++    throw / catch        错误是异常。栈展开。可以跨多层。
Anim   oi                   不是错——是挡。不堆栈。不 panic。不判断要不要恢复。
                            恢复是自动的——下一帧继续。
```

## oi 的现实语义——一声喝止

```
你正要过马路——
    朋友喊 oi。
    有车。
    你没看到车。但你停住了。
    不是因为懂了交规。是因为那一声 oi 直接剪断了你正要迈出去的运动程序。
    不需要解释。不需要理由。不需要"请处理以下异常"。
    就是——停。
```

Anim 的 oi 同构——强度 72 的 weft 正要 cross over cap 35 的 warp。oi!("intensity exceeds cap")。交叉没发生。这帧不生成信号。下一帧继续。

## 语法

```rust
fn cross_over(warp: &Warp, weft: &Weft) -> Result<CrossingPoint, oi> {
    if !warp.can_cross(&weft) {
        oi!("warp '{}' cannot cross with weft '{}'", warp.name, weft.name);
    }
    Ok(CrossingPoint::new(warp, weft))
}

// 调用方
let point = warp.cross_over(&weft, oi)?;
// oi? = 交叉被挡 -> oi 往上走。不堆栈。上一层知道"这帧没交叉"。
```

## oi 的三个特征

```
1. 不携带堆栈。
    交叉被挡只有一种原因——"没有交叉资格"。不需要追溯调用链。

2. 不 panic。不 unwind。
    oi 不是异常——是交叉路径的正常分岔。
    这帧没交叉。下一帧继续。

3. 是"挡"不是"错"。
    强度超 cap → oi。不是程序错了——是安全层在挡。
    创伤原子被禁 → oi。不是用户错了——是交叉资格的断言生效。
```

## oi 的生命周期

```
Pass 2 → oi!("intensity {n} exceeds cap {c}")  不生成 IR。
Pass 3 → oi!("trauma_v3 cannot cross with belonging")
Pass 4 → oi 在 ESIR 帧里是标记位。安全停止帧读到 -> 切保底包。
Pass 8 → oi 在 FPGA 上是寄存器比较结果。oi=true -> 紧急截断曲线。
```

## 禁止

```
❌ oi 携带堆栈——oi 不需要记忆。只需要提示。
❌ oi.panic()——不存在。oi 不是异常——是交叉路径的分岔。
❌ oi 被吞掉——oi 返回后调用方必须显式处理。无视 = 编译不过。
```
