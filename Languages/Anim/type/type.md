# Anim 类型系统

## 感受原子——双层体系

Anim 没有 `int` / `float` / `string`。Anim 的类型是感受维度。

```
核心原子 (Core Atom)
    审核      完整安全性验证（神经科学 + 精神医学 + 跨用户一致性）
    范围      全体用户，所有强度区间
    标识      「Feelings Verified ✓」
    安全约束   所有安全规则基于核心原子的验证数据
    示例      calm_meditative, grief_loss, belonging_certainty, joy_achievement

沙盒原子 (Sandbox Atom)
    审核      基础格式校验 + 恶意注入扫描，无完整安全性验证
    范围      仅限强度 ≤ 30
              仅限创作者本人及明确授权的小范围用户
              不可用于创伤协议用户
              不可用于未成年人
    标识      「Sandbox — Unverified」
    运行约束   编译时自动标注 unverified_atom 标记
              运行时安全插桩阈值加倍保守（所有生理边界 × 0.5）
              不计入跨用户一致性统计
```

两层分界不是"审核的好、没审的差"——是安全底线的精确画线。核心原子 = 锁死强度上限、全用户开放。沙盒原子 = 生态呼吸口，低强度小范围实验，不给安全系统留后门。

## 混音结构——主旋律 + 点缀

```
mix {
    main: accomplishment_satisfaction,    // 主旋律——核心感受
    accents: [
        { feeling: exhaustion_relief,  ratio: 0.25 },  // 疲惫释然
        { feeling: slight_void,         ratio: 0.15 },  // 轻微空洞
        { feeling: self_assurance,      ratio: 0.10 },  // 自我确信
    ]
}

不是 sum = 1.0 的逻辑。真实感受有溢出和消解。
点缀不是瑕疵——点缀是真实性的签名。
一点空洞 = 完成之后还有下一件事。
一点自我确信 = 这不是运气。
```

## 时间形状——感受的展开

```
shape {
    type: gradual_rise_fall,  // 渐升渐退（最常用）
    // 其他形状:
    //   abrupt_stop     —— 戛然而止
    //   wave            —— 多次起伏
    //   delayed_burst   —— 延迟爆发
    //   plateau         —— 平台维持
    //   double_peak     —— 双峰
    //   aftershock      —— 余震

    rise_duration: 90s,
    peak_duration: 30s,
    fall_duration: 120s,
}
```

形状不是时间曲线——是时间轴上的体感弧线。从起、到峰、到落——每一个阶段的导数变化决定自适应帧密度：rise 段 1ms/帧，peak 段 1ms/帧，fall 段 5ms/帧，plateau 段 10ms/帧。

## 强度——对数刻度

```
intensity [10, 30]    // 探索区。需要 cap ≥ 30。
intensity [30, 55]   // 成长区。核心价值区域。
intensity [55, 75]   // 高强度区。需要解锁 + 实时监控。
intensity [75, 90]   // 极限区。需要评估 + 知情同意。

intensity 20 -> 40 不是"两倍强"。是对数刻度——
40 的生理感受强度远超 20 的两倍。
```

## 设备类型

```
device_set {
    ear: active,        // 耳后设备——迷走神经耳支
    neck: active,       // 后颈设备——CT 纤维 + 本体感受
    wrist: active,      // 腕部设备——触觉反馈 + 皮肤电导
    temple: inactive,   // 颞部设备——EEG。未连接
    companion: active,  // 飞行陪伴体——视觉采集
}

// 缺少设备 → 体验自动降级为 outline 模式
// 交织时根据 device_set 做条件编译
```

## 安全类型——交织期强制

```
cap         承载上限——用户当前被验证的强度上限。cap 35 → intensity 必须 ≤ 35。
trauma      创伤状态——v1/v2/v3 × 社交/情绪/躯体。影响所有安全判定的元标记。
minor       未成年人标记——true → 强度上限 20。亲密维度物理隔离。
```
