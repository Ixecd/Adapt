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
