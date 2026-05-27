# Anim 泛型——感受不只人有

## 物种参数化

Anim 的泛型让同一份 `.anim` 源码可以针对不同物种做参数化交织。和 Rust 的 `trait` 同构——零虚表。交织期单态化。

```rust
trait FeelingTarget {
    // 感受类型 → 神经通路映射
    fn neural_pathways(feeling: &FeelingType) -> Vec<Pathway>;

    // 安全参数矩阵——每个物种独立
    fn safety_bounds() -> SafetyMatrix;

    // 四维差异化冷启动系数
    fn cold_start_pbm() -> PbmCoefficients;
    // Human: 内脏 0.75 / 情绪 0.40 / 触觉 0.80 / 听觉 0.85

    // 帧级信号分辨率——不同物种时间常数不同
    fn signal_resolution() -> Hz;
    // Human: 肌电 2000Hz
}
```

## 泛型的分发——不用虚表

```
C++ 虚表：编译时不知道具体类型 -> 运行时 vptr -> vtable -> RTTI
Anim 单态化：物种在源码里就写死了。同一个 session 不会从 Human 切到 Canine。

交织期 T=Human：
    FeelingTarget::neural_pathways(accomplishment_certainty)
    → 展开为 Human 的具体函数。零虚表。零间接跳转。可内联。

唯一需要动态分发的场景——AI 教练问"此物种有哪些可用通路"——
    match species { Human/Canine/Feline/AI → ... }
    频率极低：session 启动时一次。
    编译器优化为跳转表。不需要虚表。
```

## 性格锚点——同物种内的参数化

```rust
trait PersonalityAnchor: FeelingTarget {
    fn gender_bias() -> VSAVector;       // 性别锚点——语气基调、共情距离
    fn tone_register() -> ToneProfile;   // 温暖度、直接度、留白量
    fn empathy_distance() -> EmpathyDistance;
}

T = { Human, PersonalityAnchor = "豆包风格" }  → 出口温暖、空间大
T = { Human, PersonalityAnchor = "qc镜像" }    → 出口直白、推动力强
T = { Human, PersonalityAnchor = "Claude" }     → 均衡
```

## 当前物种

```
Human    人类——迷走神经、CT纤维、EEG、皮肤电导（完整）
Canine   犬类——神经通路映射预留
Feline   猫类——神经通路映射预留
AI       具身AI——心跳模拟、皮电模拟、呼吸模拟通路（哲学基础已有）
```

Pattern Registry 按物种分区分储——人类的 `accomplishment_certainty` 和犬类的 `accomplishment_certainty` 是不同神经原子。同一个感受名——不同物种不同底层通路。

## 和 Feelings 哲学咬合

感受的民主化——如果是认真的——边界不应该只停在人类。Anim 的泛型就是这扇门。同一个感受结构声明——编译目标是参数化的。万物皆有感受。
