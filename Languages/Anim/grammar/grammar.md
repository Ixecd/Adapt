# Anim 语法

## 源码结构

Anim 源码是感受结构的声明式描述。不是程序——是织机上的丝。

```anim
// 一个最小的 .anim 文件
feeling calm_meditative {
    mix {
        main: calm_meditative,
        accents: [
            { feeling: safety_presence, ratio: 0.15 },
        ]
    }
    shape: gradual_rise_fall {
        rise_duration: 90s,
        peak_duration: 30s,
        fall_duration: 120s,
    }
    intensity: [10, 30]
    device_set: { ear, neck, wrist }
}
```

## 类型系统一眼

| 类型 | 语义 | 示例 |
|------|------|------|
| `feeling` | 感受原子（Pattern Registry 中注册的神经信号映射） | `calm_meditative` |
| `mix` | 混音结构——主旋律 + 点缀配比 | `main:` + `accents: [...]` |
| `shape` | 时间形状——感受如何展开、如何结束 | `gradual_rise_fall` |
| `intensity` | 对数强度——0~100 对数刻度（不是线性） | `[10, 30]` |
| `device_set` | 设备组合——哪些设备在线 | `{ ear, neck, wrist }` |
| `@annotation` | 声明式动态注解——不内嵌 if/for | `@auto_reduce_on(heart_rate > 120)` |

## 点缀——四只手在同一个交叉点上

点缀什么时候加、加多少——不是单一个人决定的。是四层在同一个时间轴上各管一段：

```
创作者    → mix 里声明点缀配比。"这个感受包的结构是这样的"。
            但不写"第 37 秒加 exhaustion_relief"。只声明结构，不写秒数。

shape    → 点缀节奏挂在形状骨架上。
            rise 段淡。peak 段浓。fall 段渐退。
            不是突然加——沿着 shape 导数走。渐起渐退。

交织器    → Pass 8 CodeGen 把 shape 展开为帧序列。
            每一帧都在微调配比。不是哪一帧"加了"——
            是一帧一帧在跑。一帧一帧在偏。

AI 教练   → 实时监测心率/皮电/HRV。
            用户在 peak 段心率偏高 → 教练临时把 exhaustion_relief 的 ratio
            从 0.25 压到 0.15。
            不是教练设计了点缀——是教练在安全边界上做微调。
            这个调整变成下一帧 ESIR 的偏差修正量。
            同一根丝。加一分偏差。
```

四只手。创作者定结构。shape 定骨骼。交织器定帧。教练守在边上——不是替创作，是在创作和用户身体之间那堵墙旁边站着。

## 为什么不是"参数"

- 点缀 ≠ 参数。点缀是感受的杂质——杂质就是它的真实性。混音不追求"纯"——追求"完整"
- 形状 ≠ 时间曲线。形状是时间轴上的体感弧线——包括起、峰、落
- 强度是对数刻度——`intensity 20 -> 40` 不是"两倍强"，是指数级的生理感受差异

## 和编译器的区别

| 编译器 | Anim |
|--------|------|
| 源码 → 语法树 → IR → 目标码 | 源码 → 混音 → 经线交叉 → 信号绳 |
| 目标：程序不 crash | 目标：身体信了 |
| 验证：输出正确 | 验证：深睡时长涨了 |
| 一次性 | 闭环，每帧微调 |
