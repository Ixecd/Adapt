## 终结器（Finalizer）

### 设置

为对象设置关联终结函数（finalizer）

```go
// mfinal.go

func SetFinalizer(obj any, finalizer any) {
	...

	// 创建专门 goroutine，用于执行终结函数
	createfing()

	// 添加
	systemstack(func() {
		if !addfinalizer(e.data, (*funcval)(f.data), nret, fint, ot) {
			throw("runtime.SetFinalizer: finalizer already set")
		}
	})

}
```

### 添加

将相关信息打包，添加到所属 span.specials 链表内

多个 special 构成链表，按地址偏移量和类型排序。同一对象除了finalizer，还可能有 profile，都存储在该链表内

无法为同一目标对象添加多个终结器，即便终结函数不同


### 队列

在清理 span 时，检查终结器，将终结函数打包成 finalizer 放到执行队列

待执行队列 finq，由多个 finblock 构成链表。每个 finblock 用数组存储一批 finalizer 对象

### 执行

创建专门的 goroutine 用于执行终结函数

两个全局标记，分别代表唤醒和休眠

### 唤醒

在 schedule/findrunnable 里，会检查并尝试唤醒


