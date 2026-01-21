## runtine

### KeepAlived

标记目标对象存活，确保不被GC提前回收

Go的垃圾回收（GC）是基于**变量生命周期分析**的:

- 编译器会静态分析代码，计算每个变量的「最后使用点」
- 如果一个变量在代码后续不再被引用，GC可以在这个「最后使用点」之后**随时回收**它所指向的内存，即使变量还在作用域内、函数还没返回
- 这在性能优化上很有用（可以早点释放内存），但有些场景下，我们希望延长变量的生命周期，避免被过早回收

runtime.KeepAlive(obj)的官方作用:「Make the object as still reacheable until after this call」

所以如果要让对象活的更久，必须把KeepAlive放在**你希望它存活到的最后一个位置**

### ReadMemStats
获取内存分配器相关状态统计。该函数会导致STW（Stop The World），暂停所有用户线程，慎用

```go
// runtime/mstat.go

func ReadMemStats(m *MemStats) {
	stopTheWorld("read mem stats")

	systemstack(func() {
		readmemstats_m(m)
	})

	startTheWorld()
}
```

### SetFinalizer

为对象设置一个析构函数，在被GC回收时执行

当GC发现目标对象不可达时，它先解析析构函数关联，并在一个特定 goroutien 执行该函数。为了使析构函数正确执行，目标对象被重新标记为可达。等下次回收时，已没有关联析构函数，目标对象被正确回收

- 无法确定执行时间，不能保证一定被执行
- 无法保证零长度对象的析构函数被执行
- 全局变量（非堆存储）不应有析构函数
- 析构函数在单个goroutine运行，不应阻塞或执行太久
- 用 KeepAlive 标记可达位置，阻止析构函数执行
- 调用 SetFinalizer(obj, nil) 清除析构函数关联

```go
package main

import (
	"fmt"
	"runtime"
	"time"
)

func main() {
	d := make([]byte, 100<<20)
	fmt.Printf("%p\n", &d)

	runtime.SetFinalizer(&d, func(o *[]byte) {
		fmt.Printf("%p drop!\n", o)
	})

	for i := 0; i < 5; i++ {
		time.Sleep(time.Second)
		runtime.GC()
	}

	runtime.KeepAlive(&d)
}
```

循环引用会导致无法回收，导致内存泄露

### Stack

获取调用堆栈（call stack）信息

```go
func main() {
	done := make(chan struct{})

	go func() {
		defer close(done)

		buf := make([]byte, 2 << 10)
		n := runtime.Stack(buf, true)
		println(string(buf[:n]))
	}()

	<-done
}
```
