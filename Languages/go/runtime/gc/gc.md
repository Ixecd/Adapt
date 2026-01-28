## 垃圾回收

### 概述

回收线程（collector）和其他线程（mutator）可并发执行，允许多个回收线程并行

- 类型精确（type accurate）
- 非代领（non-generational）
- 非收缩（non-compacting）
- 写屏障（write barrier）
- 并发标记（mark）和清理（sweep）

回收周期包含以下几个步骤:

1. 清理结束
	- STOP-THE-WORLD，让所有P到达安全点
	- 清理所有未及时清理的内存块（清理未结束前执行 runtime.GC）
2. 开始标记
	- 状态 GCoff -> GCmark
		- 启用写屏障
		- 启用辅助（mutator assists）
		- 根标记任务排队
		- 所有 P 写屏障启用前，不扫描任何对象

	- START-THE-WORLD
		- 调度器（schedule）启动标记工人（mark workers）和分配辅助（assists）
		- 写屏障遮蔽指针写入（shades both the overwritten pointer and the new pointer value）
		- 新分配对象（malloc）直接标记为黑色

	- 执行根标记任务
		- 扫描所有栈（stack）。（会导致goroutine停止，扫描完成后恢复）
		- 遮蔽所有全局变量（global）
		- 遮蔽堆外运行时（off-heap runtime）数据结构中的堆指针

	- 清空灰色队列（drains the work queue of grey objects）
		- 扫描其中的灰色对象（grey object），将其标记为黑色
		- 对其包含的指针进行着色，将引用目标放入灰色队列

	- 终止算法（distributed termination algorithm）检测何时不再有根标记任务和灰色对象

	标记结束（gcMarkDone）

3. 标记结束
	- STOP-THE-WORLD
	- 状态 GCmarktermination，禁用标记工人和分配辅助
	- 处理内部任务（flushing mcaches）

4. 执行清理
	- 状态 GCoff，设置清理状态，禁用写屏障
	- START-THE-WORLD。从此刻起，新分配对象为白色，必要时会在分配前清理目标内存
	- 后台执行并发清理（oncurrent sweeping），并响应分配操作

5. 分配超过阈值，重启回到第一步

### 并发清理

清理与用户逻辑并发执行，专门的goroutine在后台挨个清理。标记结束后，所有span都被标记为需要清理

为避免向OS请求过多内存，首先尝试清理现有span以获取可复用空间。确保不会在未清理的 span 上执行操作，避免破坏标记位图。回收期间，mcache所持有的 span 全部回收 mcentral。重新获取时，会执行清理操作。而当下一回收周期启动时，也会先完成未清理任务

### 回收速率

环境变量 GOGC 控制了回收和分配间的线性比例。如果 GOGC = 100，使用了 4MB，那么到达 8MB 时，垃圾回收将被再次启动

### 控制器

控制器（gcController）用于GC调控，决定何时触发，有多少工作量，需要多少投入和辅助。基于每个回收周期的堆增长和CPU利用率等数据，以反馈算法（feedback control algorithm）进行调整。该算法将辅助标记和后台标记的CPU利用率优化为 GOMAXPROCS 的 25%

### 写屏障

直观上看，写屏障是编译器在用户逻辑内插入的额外指令

因三色标记和用户逻辑并发执行，那么已检查的黑色对象就可能被修改。假设已扫描黑色对象内存指针「突然」指向一个尚未扫描白色对象。按三色标记流程，黑色不会再次扫描，如此就导致该白色对象最终被回收，从而引发逻辑错误

A（黑）引用B（灰），B引用C（白色）。然后A引用C，B不再引用C。如果没有写屏障，那么A不会再次扫描，C保持白色被回收

写屏障启用后，对指针的修改会跳转到写屏障指令，以便对其重新标记、扫描。如此，其引用的白色对象会存活下来。写屏障解决了垃圾回收与用户逻辑并发执行的冲突，有助于减少重新扫描次数，简化和消除了某些复杂机制

写屏障仅在垃圾回收时启用，通过特定开关进行判断。正常情况下，用户逻辑不会跳转到这些额外指令，性能不受影响

### 三色标记

扫描内存时，使用三种颜色标记对象状态

- 白色: 潜在垃圾（尚未确认是否存活，最终会被回收）
- 灰色: 已访问到，但其引用的子对象还没完全扫描完（待处理）
- 黑色: 已完全扫描确认存活（不会被回收）

- 起初，所有对象默认为白色
- 扫描，可达对象如果包含指针，标记为灰色后放入待处理队列，否则直接黑色
- 依次从队列提取灰色对象，扫描其指针字段
	- 该灰色对象标记为黑色，表示存活
	- 其字段所引用对象，标记为灰色后放回队列
- 扫描和队列结束，仅剩黑白二色
	- 黑色为存活对象
	- 白色表示待回收空间

通过队列实现递归扫描，找出存活（黑色）对象，其余被回收。这就是三色标记原理，大体与扫描流程相对应。至于用户逻辑在扫描阶段新分配的用户对象，则直接标记为黑色

标记操作对于
```go
a := make([]*int, 1e9)
b := make([]int, 1e9)
```
性能相差巨大。因为 b 无需深入内部扫描，所以快得多。所以，对于超大内存块使用要谨慎。比如说，分配一大块[]byte 或 mmap，持有阻止回收。然后，在内部使用 uintptr 二次分配

## 初始化

垃圾回收器相关设定和初始化

```go
// mgc.go

func gcinit() {

	// No sweep on the first cycle.
	// 关闭第一次 GC 的后台清理，程序刚启动，堆里还没有任何对象，根本没东西可扫
	// 确保第一次触发 GC 时，不会误以为还有上一次的清理没做完而进入不必要的STW清理终止阶段
	sweep.active.state.Store(sweepDrainedMask)

	// Initialize GC pacer state.
	// Use the environment varable GOGC for the initial gcPercent value.
	// Go GC的「大脑」 -- 决定了什么时候触发GC、目标堆大小是多少、标记工作如何分配
	gcController.init(readGOGC())
}
```

刚启动，暂时没有后台清理工作，重点是回收控制器的初始化

```go
// mgcsweep.go

var sweep sweepdata

// State of background sweep.
type sweepdata struct {

	// active tracks outstanding sweepers and the sweep
	// termination condition.
	active activeSweep
}
```

```go
// mgcpacer.go

// gcController implements the GC pacing controller that determines
// when to trigger concurrent garbage collection and how much marking
// work to do int mutator assists and background marking.

var gcController gcControllerState
```

重新设计的 PacerRedesign 作为体检功能被开启。鉴于其尚未稳定，暂时不做深入研究


```go
func (c *gcControllerState) init(gcPercent int32) {
	// 通常是 4MB（4 << 20），是 Go 为小堆设定的最低保护阈值
	c.heapMinimum = defaultHeapMinimum

	// 新版 pacer（实验特性）
	if goexperiment.PacerRedesign {
		// PI 控制器参数
		c.consMarkController = piController{
			kp: 0.9,
			ti: 4.0,
			tt: 1000,
			min: -1000,
			max: 1000,
		}
	} else { // 旧版 pacer
		// 固定触发比例 0.875
		c.triggerRatio = 7 / 8.0
		// 表示上次GC结束后存活的堆大小
		// 假设初始最小堆4MB，触发比例0.875 -> 反推「上次存活」 大概 2.133MB
		c.heapMarked = uint64(float64(c.heapMinimum) / (1 + c.triggerRatio))
	}
	// 核心: 根据 GOGC 设置所有阈值
	c.setGCPercent(gcPercent)
}
```

初始设定的触发大小是 4MB。当然，GOGC会影响实际触发阈值

```go
type gcControllerState struct {

	// Initialized form GOGC. GOGC=off means no GC.
	gcPercent atomic.Int32

	// heapMinimum is the minimum heap size at which to trigger GC.
	// For small heaps, this overrides the usual GOGC*live set rule.
	//
	// During initialization this is set to 4MB*GOGC/100.
	heapMinimum uint64

	// triggerRatio is the heap growth ratio that triggers marking.
	//
	// E.g., if this is 0.6, then GC should start when the live
	// heap has reached 1.6 times the heap size marked by the
	// previous cycle. This should be <= GOGC/100 so the trigger
	// heap size is less than the goal heap size. This is set
	// during mark termination for the next cycle's trigger.
	//
	// Used if !goexperiment.PacerRedesign.
	triggerRatio float
}
```

```go
// setGCPercent updates gcPercent and all related pacer state.
// Returns the old value of gcPercent.

func (c *gcControllerState) setGCPercent(in int32) int32 {
	// ... 省略边界检查
	out := c.gcPercent.Load()

	if in < 0 {
		in = -1
	}

	// 根据 GOGC 调整最小堆
	c.heapMinium = defaultHeapMinimum * uint64(in) / 100
	c.gcPercent.Store(in)

	// 重点，根据 GOGC、heapMarked等重新计算
	// 下一次 GC 的触发阈值
	// 本轮GC的目标堆大小
	c.commit(c.triggerRatio)

	return out
}
```

```go
// commit recomputes all pacing parameters from scratch, namely
// absolute trigger, the heap goal, mark pacing, and sweep pacing.
//
// If goexperiment.PacerRedesign is true, triggerRatio is ignored.
//
// This depends on gcPercent, gcController.heapMarked, and
// gcController.heapLive. These must be up to date.

func (c *gcControllerState) commit(triggerRatio float64) {

	if !goexperiment.PacerRedesign {
		c.oldCommit(triggerRatio)
		return
	}

	// 目标堆 = 上次存活 * （1 + GOGC/100）
	goal := ^uint64(0)
	if gcPercent := c.gcPercent.Load(); gcPercent >= 0 {
		goal = c.heapMarked + (c.heapMarked + atomic.Load64(&c.stackScan))*uint64(gcPercent)/100
	}

	// 一系列影响触发阈值的因素参与计算 ...

	// For small heaps, set the max trigger point at 95% of the heap goal.
	// This ensures we always have *some* headroom when the GC actually starts.
	// For larger heaps, set the max trigger point at the goal, minus the
	// minimum heap size.

	minTrigger := c.heapMinimum
	maxTrigger := maxRunway + c.heapMarked

	var trigger uint64
	runway := uint64((c.consMark * (1 - gcGoalUtilizaion) / (gcGoalUtilization)) * float64(c.lastHeapScan + c.stackScan + c.globalsScan))

	if runway > goal {
		trigger = minTrigger
	} else {
		trigger = goal - runway
	}

	if maxTrigger < minTrigger { maxTrigger = minTrigger}
	if trigger < minTrigger { trigger = minTrigger }
	if trigger > maxTrigger { trigger = maxTrigger }
	if trigger > goal { goal = trigger }

	// 触发点 = 上次存活 * （1 + 0.875）
	c.trigger = trigger
	atomic.Store64(&c.heapGoal, goal)

	// Update mark pacing.
	if gcphase != _GCoff {
		c.revise()
	}
}
```

## 启动

### 触发

除因为内存分配触发阈值引起回收外，还有系统监控等引发的强制回收

```go
// mgc.go

const (
	// gcTriggerHeap indicates that a cycle should be started when
	// the heap size reaches the trigger heap size computed by the
	// controller.
	gcTriggerHeap gcTriggerKind = iota

	// gcTriggerTime indicates that a cycle should be started when
	// it's been more than forcegcperiod nanoseconds since the
	// preivious GC cycle.
	gcTriggerTime

	// gcTriggerCycle indicates that a cycle should be started if
	// we have not yet started cycle number gcTrigger.n (relative
	// to work.cycles).
	gcTriggerCycle
)
```

分配内存时，并非每次都检查。仅大块分配才会进行（shouldhelpgc），如扩容、大对象等。

```go
// malloc.go

func mallocgc(size uintptr, typ *_type, needzero bool) unsafe.Pointer {

	shouldhelpgc := false

	if size <= maxSmallSize {
		v, span, shouldhelpgc = c.nextFree(tinySpanClass)
	} else {
		shouldhelpgc = true
		span = c.allocLarge(size, noscan)
	}
}
```

测试是否达到触发条件

```go
// malloc.go

func mallocgc(size uintptr, typ *_type, needzero bool) unsafe.Pointer {
	if shouldhelpgc {
		if t := (gcTrigger{kind: gcTriggerHeap}); t.test() {
			gcStart(t)
		}
	}
}
```

```go
// mgc.go

// A gcTrigger is a predicate for starting a GC cycle. Specifically,
// it is an exit condition for the _GCoff phase.

type gcTrigger struct {
	kind gcTriggerKind
	now int64
	n uint32
}

type gcTriggerKind int
```

```go
// mgc.go

// test reports whether the trigger condition is satisfied, meaning
// that the exit condition for the _GCoff phase has been met. The exit
// condition should be tested when allocating.

func (t gcTrigger) test() bool {
	switch t.kind {
		case gcTriggerHeap:
			return gcController.heapLive >= gcController.trigger
		case gcTriggerTime:
			...
		case gcTriggerCycle:
			...
	}
	return
}
```

#### 强制回收

强制启动垃圾回收分手动和自动两种。手动使用后直接调用runtime.GC，自动则由sysmon启动

**runtime.GC**

当调用runtime.GC时，可能后台正在并行执行第N次垃圾回收。不管N处于回收周期的哪个阶段，都应该先等它结束。直到N彻底完成后，才开始N+1回收周期，直到清理（sweep）结束。注意，该函数并未调用test检查触发条件，并且会阻塞用户代码

```go
// mgc.go

func GC() {
	// 等待 N 标记结束
	n := atomic.Load(&work.cycles)
	gcWaitOnMark(n)

	// 启动 N + 1 周期
	gcStart(gcTrigger{kind: gcTriggerCycle, n: n + 1})

	// 等待 N + 1 标记结束
	gcWaitOnMark(n + 1)

	// 执行清理任务
	for atomic.Load(&work.cycles) == n + 1 && sweepone() != ^uintptr(0) {
		// 非后台清理
		sweep.nbgsweep++
		Gosched()
	}

	// 等待清理结束
	for atomic.Load(&work.cycles) == n + 1 && !isSweepDone() {
		Gosched()
	}
}
```

**sysmon**

如果垃圾回收已经长时间未执行，那么也会强制执行

```go
// proc.go

func sysmon() {
	for {
		usleep(delay)
		...

		// check if we need to force a GC
		if t := (gcTrigger{kind: gcTriggerTime, now: now}); t.test() && atomic.Load(&forcegc.idle) != 0 {
			lock(&forcegc.lock)
			forcegc.idle = 0

			// 唤醒。（将 forcegc G放回任务队列）
			var list gList
			list.push(forcegc.g)
			injectglist(&list)

			unlock(&forcegc.lock)
		}
	}
}
```

```go
// mgc.go

func (t gcTrigger) test() bool {
	switch t.kind {
		case gcTriggerHeap: ...
		
		case gcTriggerTime:
			// GOGC < 0, OFF!
			if gcController.gcPercent.Load() < 0 {
				return false
			}

			// 超过两分钟
			lastgc := int64(atomic.Load(&memstats.last_gc_nanotime))
			return lastgc != 0 && t.now - lastgc > forcegcperiod
		
		case gcTriggerCycle: ...
	}
	return true
}
```

```go
// proc.go

// 单位纳秒，2分钟
var forcegcperiod int64 = 2 * 60 * 1e9
```

### 启动

启动需要确定工作模式，并准备参与标记的工人。除此之外，还要确保前次清理结束。

```go
// mgc.go

type gcMode int

const (
	gcBackgroundMode gcMode = iota	// concurrent GC and sweep
	gcForceMode						// stop-the-world GC now, concurrent sweep
	gcForceBlockMode				// stop-the-world GC now and STW sweep(forced by user)
)
```

```go
// mgc.go

// gcStart starts the GC. It transitions from _GCoff to _GCmark (if
// debug.gcstoptheworld == 0) or performs all of GC (if
// debug.gcstoptheworld != 0).

func gcStart(trigger gcTrigger) {
	// 检查启动条件，并完成前次清理工作
	for trigger.test() && sweepone() != ^uintptr(0) {
		sweep.nbgsweep++
	}

	// 可能有多个线程检查到触发条件
	// 加锁，并再次执行条件检查，使多余的触发提前退出
	semaacquire(&work.startSema)
	if !trigger.test() {
		semrelease(&work.startSema)
		return
	}

	// 是否手动 runtime.GC 调用
	work.userForced = trigger.kind == gcTriggerCycle

	// 根据环境变量 GODEBUG=gcstoptheworld 确定回收模式
	mode := gcBackgroundMode
	if debug.gcstoptheworld == 1 {
		mode = gcForceMode
	} else if debug.gcstoptheworld == 2 {
		mode = gcForceBlockMode
	}

	// 准备 STW !!
	semacquire(&gcsema)
	semacquire(&worldsema)

	// 创建标记工人（mark goroutines）
	gcBgMarkStartWorkers()

	// 重置标记状态
	systemstack(gcResetMarkState)

	work.stwprocs, work.maxprocs = gomaxprocs, gomaxprocs
	if work.stwprocs > ncpu {
		work.stwprocs = ncpu
	}

	work.heap0 = atomic.Load64(&gcController.heapLive)
	work.pauseNS = 0 // 本周期 STW 暂停时间总计
	work.mode = mode // 回收模式

	now := nanotime()
	work.tSweepTerm = now // 前次清理结束时间
	work.pauseStart = now // 本次 STW 开启时间

	// STW !!
	systemstack(stopTheWorldWithSema)

	// 确保清理结束
	systemstack(func() {
		finishsweep_m()
	})

	// 清除 sync.Pool、sudog 等缓存
	clearpools()

	// 周期计数
	work.cycles++

	// 控制器本次周期开始
	gcContrller.startCycle(now int(gomaxprocs))
	work.heapGoal = gcController.heapGoal

	if mode != gcBackgroundMode {
		schedEnableUser(false)
	}

	// 进入并发标记阶段。（启用写屏障）
	setGCPhase(_GCmark)

	// 准备相关数据
	gcBgMarkPrepare()
	gcMarkRootPrepare()

	// 微小对象分配块被 cache 持有，所以直接加入到灰色队列
	gcMarkTinyAllocs()

	// 允许黑化标记。（启用辅助回收）
	atomic.Store(&gcBlackenEnabled, 1)

	// 解除 STW，以便 schedule 调度标记工人进行回收作业
	systemstack(func() {
		now = startTheWorldWithSema(trace.enabled) // !!

		work.pauseNS += now - work.pauseStart
		work.tMark = now
	})

	semrelease(&worldsema)

	// 非并发模式下，当前G将被暂停
	if mode != gcBackgroundMode {
		Gosched()
	}

	// 释放锁，使得其他线程可以进入
	semrelease(&work.startSema)
}
```

设置阶段状态，按需启用或禁止写屏障

```go
// mgc.go

func setGCPhase(x uint32) {
	atomic.Store(&gcphase, x)
	writeBarrier.needed = gcphase == _GCmark || gcphase == _GCmarktermination
	writeBarrier.enabled = writeBarrier.needed || writeBarrier.cgo
}
```

### 标记工人

按参与方式不同，可将标记工人分为以下几类

1. 正式工（gcMarkWorkerDedicatedMode）: 专职清理工作，不会被抢占
2. 小时工（gcMarkWorkerFractionalMode）: 参与少量工作，可被抢占
3. 临时工（gcMarkWorkerIdleMode）: 找不到其他任务，临时参与工作，可被抢占

```go
// mgc.go

type gcMarkWorkerMode int

const (
	gcMarkWorkerNotWorker gcMarkWorkerMode = iota

	gcMarkWorkerDedicatedMode

	gcMarkWorkerFractionalMode

	gcMarkWorkerIdleMode
)
```

标记未开始前，准备好与 P 等数的 worker G。待后续 gcController.startCycle 决定人数和工种

```go
// mgc.go

func gcBgMarkStartWorkers() {
	for gcBgMarkWorkderCoount < gomaxprocs {
		// 创建 worker G
		go gcBgMarkWorker()

		// 休眠，等当前这个 worker 准备好再创建下一个
		notetsleepg(&work.bgMarkReady, -1)
		noteclear(&work.bgMarkReady)

		gcBgMarkWorkerCount++
	}
}
```

新建的 worker G 保存在池内，休眠待用

```go
// mgc.go, runtime2.go

type gcBgMarkWorkerNode struct {
	// Unused workers are managed in a lock-free stack.
	node lfnode

	// The g of this worker
	gp guintptr
}

// Pool of GC parked background workers. Entries are type *gcBgMarkWorkerNode
var gcBgMarkWorkerPool lfstack
```

```go
// mgc.go

func gcBgMarkWorker() {
	gp := getp()

	node := new(gcBgMarkWorkerNode)
	node.gp.set(gp)

	// 唤醒上面 gcBgMarkStartWorkders 函数，创建下一个 worker G
	notewakeup(&work.bgMarkReady)

	for {
		gopark(func(g *g, nodep unsafe.Pointer) bool {
			node := (*gcBgMarkWorkerNode)(nodep)
			gcBgMarkWorkerPool.push(&node.node)
			return true
		}, unsafe.Pointer(node), waitReasonGCWorkerIdle, traceEvGoBlock, 0)

		// .. 标记工作细节 ...
	}
}
```