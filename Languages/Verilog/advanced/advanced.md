## advanced/ 高级主题（预留位）

对应 Go 的 advanced。初学不用碰，遇到问题再回来。

### PLL / 时钟资源
```
- 片内 PLL（GW1N 有 rPLL 硬核）
- 倍频/分频/相移：产生精准时钟
- 例：24MHz → 7.3728MHz（16×460800 波特）
```

### 存储器
```
- Block RAM（BRAM）：真双口/简单双口
- 分布式 RAM（LUT 实现）
- ROM 初始化（$readmemh 读文件）
```

### 原语 primitives
```
- IBUF/OBUF/IOBUF：IO 缓冲
- BUFG：全局时钟缓冲
- 厂商特殊原语（Gowin 的 OSCH 片内振荡器、rPLL）
```

### 高速接口
```
- LVDS/差分对
- 千兆以太网/PCIe（GW1N 没有，未来脑机平台才有）
```

### 优化
```
- 时序收敛：插流水线/改编码/约束
- 面积 vs 速度 tradeoff
- 资源利用率分析
```
