## sync

### atomic

原子操作是不可分割的操作，不会被中断，不会被其他线程干扰

原子操作一旦开始，直到结束，中间不会有任何上下文切换（context switch）。处理器通过总线锁定和缓存锁定确保原子性，让多线程不能同一时间访问相同资源

原子性不可能由软件单独保证，必须要有硬件支持，因此和架构相关。在x86平台上，CPU提供在指令执行期间对总线加锁手段。如果在汇编指令前加上LOCK前缀，其机器代码就使CPU在执行这条指令时锁住总线（或缓存锁定）。同一总线上别的CPU暂时不能通过总线访问内存，保证这条指令在多处理器环境中的原子性

**Value**

算是对原子操作的一种包装，不再局限于几种有限的数字类型

```go
// sync/atomic/value.go

type Value struct {
	v any
}

type ifaceWords struct {
	typ unsafe.Pointer
	data unsafe.Pointer
}
```

### mutex

用 state 二进制位存储状态和等待者数量

```go
type Mutex struct {
	// 1: locked
	// 2: woken		// 有人被唤醒
	// 4: starving	// 饥饿模式
	state int32
	sema uint32
}

const (
	mutexLocked = 1 << iota
	mutexWoken
	mutexStarving
	mutexWaiterShift = iota
)
```

基于原子操作和运行时信号量设计

后来者，尝试直接取锁。如果已被锁定，自选等待，避免过早休眠。如果依然失败，成为休眠等待者，并记下开始等待时间

休眠者被唤醒，循环重试（与其他人竞争），包括自旋等待。运气好，顺利取锁。运气不好，最前列继续排队

如果等待总时长超出阈值，并且再次失败，切换到饥饿模式。饥饿模式下，阻止任何人获得锁，改有由锁持有者直接移交

自旋是非公平模式，与其他人争夺，优点是效率高。饥饿模式确保公平，让等待时间不能过长，不能被饿死

释放锁会检查是否处于饥饿状态，以决定是否直接移交锁权

### rwmutex

写锁独占，必须等待读锁全部释放。读锁不互斥，可以多个读锁并存

同样是基于运行时信号量来实现的。有 RLocker 方法来返回 「读锁」 的 Locker接口，解决方法名不同的问题

```go
// sync/rwmutex.go

const rwmutexMaxReaders = 1 << 30

type RWMutex struct {
	w           Mutex	// 写锁，阻止写并发
	writerSem   uint32	// 写锁信号量，写者在获取锁时如果有读者，会在此休眠
	readerSem   uint32	// 读锁信号量，读者在获取锁时如果有写者持有锁，会在此休眠
	readerCount int32	// 读锁计数器
	readerWait  int32	// 写锁需等待读锁释放的计数器
}
```

**加锁**

读锁仅增加 readerCount，无实质性锁定。计数可能是负数，因为写者会减去常量值 -Max，使其变成负数。目的是为了阻止新读者取锁进入临界区。只能休眠，等待写者解锁时唤醒

```go
func (rw *RWMutex) RLock() {

	// 正常读锁，计数 +1，结果大于0

	// 负数只能表示写者 -Max
	// 累加计数，休眠，等待写者唤醒
	if atomic.AddInt32(&rw.readerCount, 1) < 0 {
		runtime_SemacquireMutex(&rw.readerSem, false, 0)
	}
}
```

「写者」，占住写锁，阻止其他写者进入。将读者计数器减去常量值（-Max），阻止新读者获取锁进入临界区。保存当前已锁读者数（readerWait）。休眠，等最后的读者解锁时唤醒

```go
const rwmutexMaxReaders = 1 << 30

func (rw *RWMutex) Lock() {
	// 先占写锁
	rw.w.Lock()

	// 将读者计数变成负数，阻止新读者进入临界区
	r := atomic.AddInt32(&rw.readerCount, -rwmutexMaxReaders) + rwmutexMaxReaders

	// 保存要等待的已锁读者数，休眠
	if r != 0 && atomic.AddInt32(&rw.readerWait, r) != 0 {
		runtime_SemacquireMutex(&rw.writerSem, false, 0)
	}
}
```

**解锁**

读者解锁时，递减读者计数器。如果结果为负数，表示有写者在等待。递减等待计数。最后一位读者解锁，唤醒写者

```go
func (rw *RWMutex) RUnlock() {
	// 如果读计数器是负数，那么有写者在等待
	if r := atomic.AddInt32(&rw.readerCount, -1); r < 0 {
		rw.rUnlockSlow(r)
	}
}

func (rw *RWMutex) rUnlockSlow(r int32) {
	// 非法检查: 如果递减前 readerCount == 0（无锁）或 == -rwmutexMaxReaders（写者持有但无等待读者）
	// 说明 RUnlock 调用次数超过了 RLocks，属于Bug
	// r + 1 == 0；递减前readerCount == 0，说明本来就没有读锁，却调用了RUnlock
	// r + 1 == -rwmutexMaxReaders: 递减前 readerCount == -rwmutexMaxReaders，说明写者持有锁，但无等待读者，同样说明多调用了RUnlock属于Bug
	if r + 1 == 0 || r + 1 == -rwmutexMaxReaders {
		race.Enable()
		throw("sync: RUnlock of unlocked RWMutex")
	}

	// 原子递减写者等待的读者计数
	// 如果结果 == 0，说明这是最后一位读者退出
	if atomic.AddInt32(&rw.readerWait, -1) == 0 {
		runtime_Semrelease(&rw.writerSem, false, 1)
	}
}
```

写者解锁时，将读者计数加上常量值（+Max），其结果就是休眠等待的读者数。唤醒所有休眠读者，释放写锁

```go
func (rw *RWMutex) Unlock() {
	// 原子加回大常量
	// 这会把负的 readerCount 恢复成正数，结果 r 就是 写者持有锁期间积累的等待读者数
	r := atomic.AddInt32(&rw.readerCount, rwmutexMaxReaders)

	// 非法检查: 如果恢复后的计数 >= rwmutexMaxReaders（1<<30），说明等待的读者数太多，属于Bug
	if r >= rwmutexMaxReaders {
		race.Enable()
		throw("sync: Unlock of unlocked RWMutex")
	}

	for i := 0; i < int(r); i++ {
		runtime_Semrelease(&rw.readerSem, false, 0)
	}

	rw.w.Unlock()
}
```

### waitgroup

允许有多个等待，通过 `Add/Done` 和 `Wait` 两个计数器来判断状态

```go
// sync/waitgroup.go

type WaitGroup struct {
	noCopy noCopy

	state1 uint64
	state2 uint32
}

func (wg *WaitGroup) state() (statep *uint64, semap *uint32) {
	return &wg.state1, &wg.state2
}
```

累加或递减计数，归零时唤醒所有等待者

```go
func (wg *WaitGroup) Done() {
	wg.Add(-1)
}

func (wg *WaitGroup) Add(delta int) {
	// 累加 Add 计数
	statep, semap := wg.state()
	state := atomic.AddUint64(statep, uint64(delta)<<32)

	v := int32(state >> 32)		// add.count
	w := uint32(state)			// wait.count

	if v < 0 {
		panic("sync: negative WaitGroup counter")
	}

	if w != 0 && delta > 0 && v == int32(delta) {
		panic("sync: WaitGroup is reused before previous Wait has returned")
	}

	if v > 0 && w == 0 {
		return
	}

	if *statep != state {
		panic("sync: WaitGroup misuse: Add called concurrently with Wait")
	}

	*statep = 0
	for ; w != 0; w-- {
		runtime_Semrelease(semap, false, 0)
	}
}
```

如果计数为0，不需要等待。否则，累加等待计数，休眠

```go
func (wg *WaitGroup) Wait() {
	statep, semap := wg.state()
	for {
		state := atomic.LoadUint64(statep)

		v := int32(state >> 32)
		w := uint32(state)

		if v == 0 {
			return
		}

		if atomic.CompareAndSwapUint64(statep, state, state + 1) {
			runtime_Semacquire(semap)
			return
	}
}
```

### cond

发送单播和广播事件，解除等待

```go
// sync/cond.go

type Cond struct {
	noCopy noCopy

	L Locker

	notify notifyList
	checker copyChecker
}

func NewCond(l Locker) *Cond {
	return &Cond{L: l}
}
```

### once

仅执行一次，与执行目标无关

```go
// sync/once.go

type Once struct {
	done uint32
	m    Mutex
}

func (o *Once) Do(f func()) {
	// 检查执行标记，执行与否与参数 f 无关
	if atomic.LoadUint32(&o.done) == 0 {
		o.doSlow(f)
	}
}

func (o *Once) doSlow(f func()) {
	o.m.Lock()
	defer o.m.Unlock()

	if o.done == 0 {
		defer atomic.StoreUint32(&o.done, 1)
		f()
	}
}
```

**双重检查锁定**

- 在 `Do` 里的 `if` 检查，可有效阻止参与者进入临界区，提高效率
- 而 `doSlow` 第二个 `if` 要应对的是，首次调用有多个并发通过检查进入临界区，确保后续锁持有者，不会再次执行

### map

只有两种情况下，可减少锁争用，提升性能

- 少写多读
- 多个并发读写不同项

快慢分离，read 以 atomic 操作，dirty 需 mutex 锁定。相比读写锁（rwmutex）写操作锁定全局，此设计有更好性能

查找优先选择 read，失败后再以慢路径从 dirty 找。不直接存储值，而是以 entry.p 引用，所以可用原子操作修改。删除也不过是 entry.p = nil，不影响字典本身

慢操作累加 misses，达到阈值时，以 dirty 替换 read 字典。然后按需重建 dirty 字典，并复制 read 里所有非删除项

```go
// sync/map.go

type Map struct {
	mu Mutex

	read atomic.Value	// readOnly
	dirty map[any]*entry

	misses int
}

type readOnly struct {
	m map[any]*entry
	amended bool
}

type entry struct {
	p unsafe.Pointer
}
```

### pool

并发安全对象池。通过复用，减少临时对象，降低垃圾回收压力。池内部闲置对象（无外部引用）可被垃圾回收，无通知

每个 `Pool` 实例管理多个 `poolLocal` 与，`P` 对应。动态数组 `[localSize]poolLocal`，以`P.id`为索引

```go
// sync/pool.go

type Pool struct {
	noCopy noCopy

	// 每个 P 拥有一个本地缓存，实际上的类型为 [P]poolLocal
	local unsafe.Pointer
	localSize uintptr

	// 每次垃圾回收，都会将 local 缓存
	// 改成 victim 受害缓存，以便下轮回收
	// 当 local 找不到时，也会尝试受害缓存

	victim unsafe.Pointer
	victimSize uintptr

	New func() any
}
```

**缓存**

相关操作通过 `pin` 返回当前 `P` 所属 `poolLocal` 缓存。该方法会初始化`Pool.local`内存，并添加到 `allpools` 全局列表

**获取**

每个 `P.poolLocal` 内有两处缓存

- private: 私有。仅一个闲置对象，快速分配
- shared: 共享。可能被其他 P 偷窃

优先级: private > shared > steal > new

**放回**

所获取对象已从 `Pool` 移除，需显式放回

```go
func (p *Pool) Put(x any) {
	if x == nil {
		return
	}

	// private
	l, _ := p.pin()
	if l.private == nil {
		l.private = x
		return
	}

	// shared
	if x != nil {
		l.shared.pushHead(x)
	}
	
	runtime_procUnpin()
}
```

**清理**

当 GC 启动时，调用 `clearpools` 清理对象池

### copycheck

嵌入到其他结构里，检查对象是否被复制

```go
// sync/codn.go

type copyChecker uintptr

func (c *copyChecker) check() {
	
	if uintptr(*c) != uintptr(Pointer(c)) && !CompareAndSwap((*uintptr)(c), 0, uintptr(Pointer(c))) && uintptr(*c) != uintptr(Pointer(c)) {
		panic("sync.Cond is copied")
	}
}
```

作为一个整数，`copyChecker` 存储自己的指针

**首次调用**:

- `c != *c`成立
- `CAS(c, 0, c)`成功将自己指针写入
- `!CAS`短路，不会继续执行

**后续调用**:

- `c != *c`直接短路

如果被拷贝，那么这个数据也被复制。对比相关条件，最终触发panic

- `c != *c` 成立，通过
- `CAS` 失败，但`!CAS成立，通过
- `c != *c` 再次通过，触发`panic`

### nocopy

匿名嵌入，可让`go vet`（Go官方提供的「静态代码分析工具」，强烈推荐加到 CI pipeline中）检查到复制行为

```go
// sync/cond.go

type noCopy struct {}

// 实现 sync.Locker 接口
func (*noCopy) Lock() {}
func (*noCopy) Unlock() {}
```