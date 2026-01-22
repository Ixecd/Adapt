## time

**时区**

- GMT: 格林尼治标准时间
- UTC: 世界标准时间，误差必须保持在0.9秒内。由巴黎的国际地球自转事务中央局负责修正
- CTT: 东八区
- CST: 中国标准时间，UTC+8

**时间戳**

- Unix时间戳: 1970年1月1日00:00:00 UTC以来的秒数
- UnixNano时间戳: 1970年1月1日00:00:00 UTC以来的纳秒数

### Duration

纳秒精度的时间段

```go
type Duration int64

const (
	Nanosecond  Duration = 1
	Microsecond          = 1000 * Nanosecond
	Millisecond          = 1000 * Microsecond
	Second               = 1000 * Millisecond
	Minute               = 60 * Second
	Hour                 = 60 * Minute
)

func main() {
	d, _ := time.ParseDuration("1h30m15s100us")
	fmt.Println(d)

	d, _ = time.ParseDuration("1us1ns")
	fmt.Println(d.Nanoseconds())
}
```

### Time

纳秒精度的时间

- Date: 按参数构造
- Now: 当前系统本地时间
- Parse: 按格式解析字符串
- Unit: 基于 1970年1月1日 00:00:00 UTC 的纳秒数

### Timer

定时器核心由运行时实现，因为要配合调度器工作

- Timer: 单次事件
- Ticker: 周期事件

