## 内存分配

内存分配器相关参数定义

```go
// sizeclass.go, malloc.go

const (
	pageShift = _PageShift
	pageSize  = _PageSize

	_PageShift = 13
	_PageSize  = 1 << _PageShift	// 8kb
)
```

以 32KB 为分界，将对象分为大小两类

```go
// sizeclass.go

_MaxSmallSize = 32768
_NumSizeClasses = 68
```

其中小对象按8字节对齐，再分成67种（1-67）（0表示large objects）

使用静态表进行大小（bytes）和类别（class）转换

### 地址空间

通过「预留」方式获取连续地址空间，以便后续合并内存块，减少碎片

**基本概念**

- arena: 预留地址空间，用户对象在此范围内分配
- bitmap: 基于类型信息，以位图标记对象指针
- spans: 反查内存归属的 mspan 管理对象

### 可分配地址

每个arenaHint记录可分配起始地址（累进）及分配方向（向高位或低位）。如果分配失败，则尝试从链表获取下一个arenaHint重试，或由操作系统提供随机地址

### 已分配空间

用数组管理多个 heapArena，每个对应一到多块内存，在linux/amd64下，每个heapArena对应 64MB 的内存空间

位图（bitmap）以及反查表（spans）大小计算，以linux/amd64为例

- L1 长度为 1
- L2 可容纳 4MB = 4194304 个heapArena指针

每个heapArena管理64MB，总容量可达 `1 * 4194304 * 64MB = 256TB`
理论上能覆盖地址空间，没有分配区域为nil

### 流程

上层部件（mcentral.grow、largeAlloc）调用 mheap.alloc 从堆获取内存。期间，调用 setSpans 填充反查表。而heapArena是在扩张（grow）使调用sysAlloc创建

### 堆内存分配

- 向操作系统申请内存
- 为上层部件分配内存
- 管理正在使用和闲置内存
- 回收不再使用的内存

分配、重用和回收管理，由独立单元 pageAlloc 完成

**相关元数据**

- summary: 用于查找可用内存块的摘要
- chunks: 标记使用和释放状态的位图

所谓「摘要（summary）」，就是将内存块（chunk）的start、end、max打包成一个整数

使用位图（pallocBits），意味着堆内部不再使用mspan管理

### 分配

分配操作获取内存，返回 mspan 对象，并初始化相关属性

核心是 pageAlloc.alloc 操作

- 基于摘要查找可用内存块
- 更新位图信息

### 回收

向堆归还内存块

所有数据都以元数据进行管理，所以释放也只需清除位图标记，同时更新摘要即可

### 系统内存申请

调用系统包装函数

Linux使用 Huge Page 方式

Huge Page: 大内存页，相比普通页（4KB），更加高效。无需交换（swap）。通过更大页尺寸，减少映射表条目（TLB）

Windows 内存分配发生在 sysUsed 调用，以便从堆中提取内存（allocSpan）时可以补上已释放内存

### 分配

按对象大小，选择不同分配策略

#### 零长度对象

堆上分配的零长度对象都指向同一个全局变量，以获得合法内存地址

即便是不同类型的零长度对象（堆分配），都会指向同一位置

#### 大对象

直接从堆提取大小合适的内存块

#### 小对象

在 mcache 里使用数组存储不同类别（sizeclass）的span内存块

**索引**

数组（mcache.alloc）长度是类别（sizeclass）的两倍

每种类别按是否包含指针分scan和noscan两种，有助于GC优化

**提取**

每个 mspan 仅为一种 sizeclass 服务，所以内部 object 等长

依据 allocBits 位图，将整个 mspan 内存当做 object 数组对待，通过索引计算偏移量

算法 nextFreeIndex 的关键是freeIndex和allocCache工作方式

- freeIndex: 存储上次扫描位置，本次扫描以此为起点
- allocCache: 缓存 freeIndex 后的部分数据，以提高扫描效率

优先使用快速版本，仅检查缓存。没有无关调用，不填充，不扩容，一切为了性能

**扩容**

如果nextFree发现mspan没有剩余空间，那么从central扩容

与mcache类似，mheap.central数组同样包含scan、nboscan两类

每个 mcentral 按是否有剩余空间，管理partial和full列表。另外，partial和full都有两个集合，分别表示已清理（swept）和未清理（unswept）

扩容时，依次尝试 partial swept、partial unswept、full unswept集合，直到从heap重新分配

鉴于期间清理操作动作较大，为避免花费太多时间，采取了尝试总次数限制

#### 微小对象

微小对象（tiny）长度小于16字节，最常见的就是小字符串。将多个微小对象组合起来，用单object存储，可有效减少内存浪费

```go
// malloc.go

maxTinySize = _TinySize
tinySizeClass = _TinySizeClass

_TinySize = 16
_TinySizeClass = int8(2)
```

因垃圾回收的缘故，用来组合的微小对象不能包含指针。直到单元里所有微小对象都不可达时，该内存才会回收

通过偏移位置（tinyoffset）可判断剩余空间是否满足需求。如果可以，以此计算并返回内存地址。不足，则提取新内存块，返回起始地址便可。追后，对比新旧两块内存，留下剩余空间更大的那块

### 回收

内存回收通常由垃圾回收器引发，内存分配器具体执行

**缓存**

垃圾回收将所有 P.cache 持有的 mspan 交还给 central，以便闲置内存可以调度给其他P.cache使用

由自己 M/P 执行，因此不会和分配操作冲突

当mspan从cache交还给 central 时，其内存可能尚有剩余。如此，其他 P.cache 获取该 mspan 时，就能使充分利用剩余内存。一个mspan可悲多个cache使用，但同一时刻仅有一个使用者，不存在竞争

至于 emptymspan，仅用于占位

收归 central 时，根据是否有剩余空间来决定放入哪个列表

**清理**

垃圾标记完成后，以span为单位进行清理（sweep）操作

**二级平衡**

首先，将 P.cache.alloc 数组内 mspan 上交 central，有剩余内存的 mspan 可调度给其他 P.cache 使用。此第一级平衡，避免P长时间闲置内存

其次，将收回全部空间的 mspan 从 central 交还给 heap，那么该 mspan 可以调剂给其他 central 使用。无非重制属性，诸如 spanclass之类的，完全没有影响。这就是二级平衡，避免central长时间闲置内存


### 释放

#### 同步释放

调用 scavenge 主动释放闲置物理内存（RSS）

**两个前提**

首先，向操作系统申请内存时，会将地址段保存到 mheap.alloc.inUse 内

其次，每轮释放操作前调用 scavengeStartGen 初始化相关状态，重点是复制 inUse 地址段

接下来，依次取地址段尝试物理内存释放，直到满足需求

最终，通过以 sysUnused 完成物理内存释放

释放操作仅针对物理内存，也就是解除物理内存和虚拟内存的映射。分配器管理的虚拟内存并未被释放，毕竟每个进程有256TB的虚拟内存空间，且「不占用」物理内存，完全没必要释放虚拟内存。等该虚拟内存被复用时，会检查其释放标志，调用 sysUsed 重新关联谁给你物理内存

**触发位置**

当堆内存扩张时，会尝试释放「等量」闲置物理内存（碎片），以避免浪费

**手工释放**

用户调用 runtime/debug.FreeOSMemory，会释放全部闲置物理内存

#### 异步释放

后台异步释放由独立 goroutine 执行

该 G 以循环方式执行，单次释放足量内存。如释放没反应，表示当前没有「多余」物理内存，阻塞后等待手工唤醒，否则以定时器唤醒

系统监控（sysmon），以及清理结束（finishsweep_m）时，调用 wakeScavenger 唤醒后台操作

注意，每次唤醒操作都会停止计时器。只有释放量大于0时，才会进入定时休眠。期间，会重置定时器，以便重新唤醒

#### 物理内存

Unix-lick系统以 madvise 建议内核解除物理内存映射。从而在保留虚拟地址的情况下，达到释放物理内存的目的。当这些内存被重新使用时，引发缺页异常，由内核自动补齐所需要物理内存

madvise只是建议，内核未必执行或立即执行

使用 MADV_FREE 代替 MADV_DONTNEED 以获得更好的提升。可用 GODEBUG=madvdontneed=1 开启

```go
// mem_darwin.go

func sysUnused(v unsafe.Pointer, n uintptr) {
	madvise(v, n, _MADV_FREE_REUSABLE)
}

func sysUsed(v unsafe.Pointer, n uintptr) {
	madvise(v, n, _MADV_FREE_REUSE)
}

// mem_windows.go

func sysUnused(v unsafe.Pointer, n uintptr) {
	r := stdcall3(_VirtualFree, (uintptr)v, n, _MEM_DECOMMIT)
}

func sysUsed(v unsafe.Pointer, n uintptr) {
	p := stdcall3(_VirtualAlloc, (uintptr)v, n, _MEM_COMMIT | _MEM_RESERVE, _PAGE_READWRITE)
}
```