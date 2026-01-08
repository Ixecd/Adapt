## 性能监控
采集测试或运行数据，分析问题，针对性改进代码

**目标类型**

cpu、alloc、heap、threadcreate、goroutine、block、mutex

**采样方式**

- 测试: `go test -memprofile mem.out`
- 在线: `import _ "net/http/pprof"`
- 手工: `runtime/pprof`

## 测试采样
`go test -run NONE -bench . -memprofile mem.out net/http`
- `-cpuprofile`: 执行时间
- `-memprofile`: 内存分配
- `-blockprofile`: 阻塞
- `-mutexprofile`: 锁争用
- `memprofilerate`: runtime.MemProfileRate
- `blockprofilerate`: runtime.SetBlockProfileRate
- `mutexprofilefraction`: runtime.SetMutexProfileFraction

命令行、服务、交互三种模式查看采样结果

```bash
go tool pprof -top mem.out
go tool pprof -http 0.0.0.0:8080 mem.out # 根据服务，推荐
go tool pprof http.test mem.out			# 交互
```
- `flat`: 仅当前函数，不包括它调用的其他函数
- `sum`: 列表前几行所占百分比总和
- `cum`: 当前函数调用堆栈累计

推荐还是去web看吧

## 在线采样
向目标程序注入`net/http/pprof`包

```go
package main

import (
	"net/http"
	// 这个包的init函数自动向系统默认的HTTP处理器注册路由
	// 对程序性能的影响几乎可以忽略不计，可以放心的在生产环境中使用
	// 简化后的 net/http/pprof 源码逻辑
	// func init() {
	// 	http.HandleFunc("/debug/pprof/", Index)
	// 	http.HandleFunc("/debug/pprof/cmdline", Cmdline)
	// 	http.HandleFunc("/debug/pprof/profile", Profile)
	// 	http.HandleFunc("/debug/pprof/symbol", Symbol)
	// 	http.HandleFunc("/debug/pprof/trace", Trace)
	// }
	_ "net/http/pprof"
)

func main() {
	http.ListenAndServe(":8080", http.DefaultServeMux)
}
```

```bash
curl http://localhost:8080/debug/pprof/heap -o mem.out
go tool pprof mem.out
```
## 手工采集

```go
package main

import (
	"runtime/pprof"
	"os"
)

func main() {
	pprof.StartCPUProfile(os.Stdout)
	defer pprof.StopCPUProfile()

	// ...

}
```
## 执行跟踪
相比`profile`采样统计，`trace`捕获一个时段内的执行事件

```bash
go test -trace trace.out net/http			# 采样
go tool trace -http 0.0.0.0:8080 trace.out	# 服务
```