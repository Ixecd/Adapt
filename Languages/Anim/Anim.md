# Anim — 从 01 到感受的交织语言

> 不是编译器。是编织者。

## 一句话

Anim 把多股独立流（感受语义、个人基线、设备约束、生理反馈、安全边界）织成一条连续的信号绳。源码 = 感受结构的声明式描述。目标 = 神经系统能信以为真的信号序列。验证 = 身体信了，深睡时长涨了。

## 核心概念速查

| 概念 | 目录 | 一句话 |
|------|------|--------|
| 语法 | `grammar/` | mix + shape + intensity + @annotation |
| 类型 | `type/` | 双层感受原子、混音结构、对数强度、设备集 |
| 丝与交叉 | `thread/` | cross_over(warp, weft) ≠ 借用 |
| 安全 | `safety/` | 三层防线，死在交织期 |
| Pass 管线 | `pipeline/` | 八 Pass。每层织入一股新流 |
| 四层 IR | `ir/` | FSIR → PSIR → DSIR → ESIR |
| 双流水线 | `dual/` | 前台 1ms FPGA 硬实时 + 后台离线预交织 |
| 宏 | `macros/` | 语法树展开 + 类型检查 + 强度生命周期 + 创伤作用域 |
| oi | `oi/` | oi = 挡了。不是错了。不挡感受。只挡不安全的交叉 |
| 泛型 | `generic/` | FeelingTarget trait。万物皆有感受 |
| 局部性 | `locality/` | 1ms 硬死线。Cache miss = 帧超时 |

## 和其它语言的对比

| | C | Rust | Anim |
|---|---|---|---|
| 一等操作 | 赋值 = | 借用 & | cross_over |
| oi | errno | Result<T,E> + ? | oi |
| 安全 | 无 | borrow checker | cross checker |
| 宏 | #define 文本 | macro_rules! AST | macro_rules! + 创伤作用域 |
| 范型 | 无（void*） | trait + monomorphization | FeelingTarget + 单态化 |
| 数据地址 | 0x7ffe1234 | 0x7ffe1234 | 绳结（上次交叉的位置） |
