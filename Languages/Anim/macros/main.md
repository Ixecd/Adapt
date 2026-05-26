# Anim 宏——交织期展开的三层保证

## 不是 C `#define`——是语法树级展开

```
C #define:      文本替换。不安全。无作用域。无类型。
Rust macro_rules!: 语法树展开。安全。有作用域。有类型。
Anim macro_rules!: 语法树展开 + 感受类型检查 + 强度生命周期 + 创伤作用域。
```

## 语法

```anim
macro_rules! safe_grief_accent {
    () => {
        { feeling: grief_loss, ratio: 0.08 }
    };
}

mix {
    main: belonging_certainty,
    accents: [
        safe_grief_accent!(),  // 交织期展开为 AST 节点。不是字符串替换。
    ]
}
```

展开时机：Pass 0 解析后、Pass 1 类型检查前。展开是语法树替换——Pass 1 看到的是展开后的完整 AST，和手写的无区别。

## 三层保证

```
1. 感受类型检查
    safe_pair!(explosion_rage, calm_meditative)
    → Pass 1 检测到组合情绪对冲 → 编译警告。
    调用方必须显式 #[allow_hedge] 才能通过。

2. 强度生命周期
    macro 定义 intensity [60, 75]。
    调用方 cap 35 → Pass 2 直接拒绝。不生成信号。
    同一宏——不同用户不同 cap 下不同结果。不是宏的问题。

3. 创伤作用域
    #[trauma_scope(v1, v2)]
    macro_rules! gentle_belonging { ... }
    此宏的作者声明"为 v1 和 v2 做了安全验证"。
    v3 不在作用域 → Pass 3 拒绝。
    无标注 = 默认仅 trauma: none 开放。安全默认。
```

## 作用域

```
文件级    macro_rules! { ... }     仅当前 .anim 文件可见
包级导出  pub macro_rules! { ... }  被其他 .anim 文件 import
导入      use feeling_package::name;  和 Rust 语义一致
递归深度  最大 32 层——和 Rust 一致
```

## 禁止

```
❌ C #define 文本替换——不存在于 Anim。
❌ 宏访问用户 PBM——宏在 Pass 0 后展开。PBM 在 Pass 6 才参与。
❌ 宏绕过安全检查——展开后必经 Pass 1/2/3。不存在"内联不安全"。
❌ 宏依赖运行期数据——宏展开在交织期。外部数据在运行期。不可交叉。
```
