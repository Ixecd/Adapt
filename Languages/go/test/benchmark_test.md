## 基准测试

有时也称为性能测试。目的是获知算法执行时间，以及内存开销。

- 保存在 `_test.go` 文件中
- 函数以 `Benchmark` 为前缀
- 类型 `B` 与 `T` 方法类似，省略

- 以 `go test -bench` 执行
- 仅执行性能测试，可用`-run NONE` 忽略单元测试

- `-bench` 指定要运行的测试
- `-benchtime` 指定单次测试运行的时间或循环次数(默认1s， 1m30s， 100x)
- `-count` 执行几轮测试
- `-cpu` 测试所用 CPU 核心数，如果列出来多个数字，则Go会为列表中的每个数字运行一遍完整的基准测试
- `-list` 列出测试函数，不执行
- `-benchmem` 显示内存分配(堆)信息

### 内部实现
内部通过增加循环次数，直到取样(时间或次数上限)足够，以获得最佳平均值

决定循环次数(`b.N`)的因素，按优先级次序
- 手动指定次数(`-benchtime 10x`)
- 内部次数上限(`1e9`)
- 手动指定时长(`-benchtime 10s`)

性能测试会执行`runtime.GC` 清理现场，以确保测试及结果不受干扰

### 子测试

操作与`T`基本一致。但没有`Parallel`，而是`RunParallel`

### 计时器
计时器默认自动处理。如测试及逻辑中有需要排除的因素，可手工调用

```go
func BenchmarkTimer(b *testing.B) {
	// setup
	time.Sleep(1 * time.Second)

	// teardown
	defer time.Sleep(1 * time.Second)

	// 这里要重置计时器，避免前面的setup影响测试时间
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		add(1, 2)
	}
	// 停止计时器，避免teardown干扰
	b.StopTimer()
}
```

### 并行
方法`b.RunParallel` 创还能多个goroutine并发测试单个目标
- 不能操作计时器(StartTimer/StopTimer/ResetTimer)
- 不能执行子测试

```go
func BenchmarkA(b *testing.B) {
	b.RunParallel(func(pb *testing.PB) {
		for pb.Next() {
			time.Sleep(1 * time.Second)
		}
	})
}
```

### 内部实现
将总次数(`b.N`, `BN`) 按粒度(`grain`,`~100us`) 拆分，每个goroutine每次取一个分段执行。如总任务未完成，则再取一个分段

使用计数器(`cache`)记录当前分段任务内部进度。每次调用`PB.Next`，计数器递减。归零时，将此次分段计入总进度(`globalN`)。检查总进度，判断是否要再取分段

### 内存
除执行时间外，还应该关注堆内存分配。因为内存分配和垃圾回收都数据重点性能问题

```go
func BenchmarkMem(b *testing.B) {
	// b.ReportAllocs() 调用这个，命令行中没有 -benchmem也会输出内存信息
	for i := 0; i < b.N; i++ {
		_ = make([]byte, 1 << 20)
	}
}
```

`go test -v -run None Mem -benchmem ./mylib`

