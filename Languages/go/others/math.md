## math

常用的是 「整数」 和 「浮点数」 最大最小值定义

```go
const (
	MaxInt		= 1 << (intSize - 1) - 1
	MinInt		= -1 << (intSize - 1)
	MaxInt8		= 1<<7 - 1
	MinInt8		= -1 << 7
	MaxUint64	= 1<<64 - 1

	MaxFloat32	= 0x1p127 * (1 + (1 - 0x1p-23))
	MaxFloat64	= 0x1p1023 * (1 + (1 - 0x1p-52))
)
```

## rand

伪随机性（pseudo random）是指一个过程似乎是随机的，但实际上并不是

伪随机数使用确定性算法计算出「随机」数序。如果种子（seed）不变，那么返回伪随机数序也不变

如果注重安全，建议使用`crypto/rand`包。或从`/dev/random`，`/dev/urandom`读取数据进行处理，前者会阻塞

- Perm: 返回数列切片（乱序）
- Read: 填充字节切片（byte）
- Shuffle: 洗牌

