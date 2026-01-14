## context
**上下文**用于广播**消息**和传递**数据**

- **链式流程**: 每个节点都可检查消息状态，以决定是否放弃后续调用
- **并发模型**: 创建并发任务单元，给予额外协作控制（取消、超时）

- 消息只能向后广播，并且只是建议，非强制
- 数据沿调用链传递，子知父态，反之不行

```go
type Context interface {
	// 截止时间: 未设置时， ok == false
	Deadline() (deadline time.Time, ok bool)

	// 消息通知: closed (cancel, deadline, timeout)
	Done() <- chan struct{}

	// 取消原因: nil, Canceled, DeadlineExceeded
	Err() error

	// 关联数据: 不适合传递业务参数
	Value(key any) any
}
```

消息 = `Done + Err`

- Done: 检查消息是否发生
- Err: 确认是哪类消息

**说明**

- 上下文对象不可变，以 With 函数生成有继承关系的子对象
- 以 Background 或 TODO 为根对象，以ctx为参数名
- 以 Value 方法获取关联值时，逐级向上（父）递归查找
- 所有 cancel 函数应显式调用，避免正常结束未及时释放资源

### cancel

向下（子）发送「取消广播」

- 所谓消息，就是 done 被关闭
- 即便正常结束，也应该调用 cancel 函数清理资源
- 内部同步保护，多次调用 cancel 没有影响

```go
type cancelCtx struct {
	Context		// parent

	mu 			sync.Mutex
	done 		atomic.Value			// channel
	children 	map[canceler]struct{}	// broadcast
	err 		error
}
```

- Context: 直属父辈，用于往上递归
- children: 广播字典。后辈可能跳过几层 Context，找到并加入

所有 With 函数都返回一个包装上下文，以 Context 嵌入字段记录父级上下文。WithCancel 额外返回一个 cancel 函数，其中包括 Err 返回值

```go
func WithCancel(parent Context) (ctx Context, cancel CancelFunc) {
	c := newCancelCtx(parent)
	propagateCancel(parent, c)
	return &c, func() { c.cancel(true, Canceled) }
}

func newCancelCtx(parent Context) cancelCtx {
	return cancelCtx{Context: parent}
}
```

创建时，尝试加入父辈广播网(children)，以便接收消息。往上递归查找 cancelCtx 类型祖先，只有它有广播字典(children)。当cancel被调用，广播（递归）消息

### timer
这是 `WithTimeout`、`WithDeadline` 的内核。就是继承 `cancelCtx` 手动取消功能后，另加定时器自动执行

```go
type timerCtx struct {
	cancelCtx

	timer *time.Timer
	deadline time.Time
}
```

### value
以递归方式通过层级关系向上（父）查找

```go
type valueCtx struct {
	Context
	key, val any
}
```

```go
func (c *valueCtx) Value(key any) any {
	if c.key == key {
		return c.val
	}
	return c.Context.Value(key)
}

```