package main

import (
	"context"
	"fmt"
	_ "os"
	"reflect"
	"runtime"
	"sync"
	"time"
	"unsafe"
)


func a() {
	println("a")
}

func b() {
	println("b")
	runtime.Goexit() // 退出当前 goroutine, 不会执行c
}

func c() {
	println("c")
}

func concurrencyTest() {
	q := make(chan struct{})

	go func() {
		defer close(q)
		fmt.Println("done.")
	}()

	<-q

	println("===============>")

	var wg sync.WaitGroup
	wg.Add(10)

	for i := 0; i < 10; i++ {
		go func(id int) {
			defer wg.Done()
			fmt.Println(id,"done.")
		}(i)
	}

	wg.Wait()

	println("===============>")

	// 创建一个可取消的 Context
	// context.Background() 是根 context，通常作为最顶层的父context
	// ctx: 一个 Context 对象，带有 Done() 方法返回一个只读通道 (<-chan struct{})
	// cancel: 一个无参数的取消函数，调用它会关闭内部的 done 通道，并向所有从这个ctx派生的子 context 传播取消信号
	ctx, cancel := context.WithCancel(context.Background())
	
	go func() {
		// 立即关闭ctx的内部 done 通道
		// 向所有监听 ctx.Done() 的地方发送"取消/完成"信号
		defer cancel()
		println("done.")
	}()
	// 主 goroutine 从ctx的done通道接收信号
	// 这个接收操作会阻塞,知道有人调用 cancel() 或者 父context被取消
	// 一旦 cancel() 被调用, <-ctx.Done() 会立即返回(通道关闭),主gorotine继续执行
	<-ctx.Done()

	println("===============>")
	// 用锁实现同步，有严重隐患，不推荐，现代GO版本 + 简单程序中 几乎不会出现死锁
	// 推荐
	// 谁Lock，谁就Unlock
	// 谁启动goroutine，谁就等待它结束
	var lock sync.Mutex
	
	lock.Lock()

	time.Sleep(time.Millisecond)

	go func() {
		time.Sleep(time.Millisecond)
		defer lock.Unlock()
		println("mutex locked.")
	}()
	// main goroutine 在第二次 Lock() 阻塞后, runtime 会 "确保" 唤醒其他 goroutine来推进系统。
	// go 1.17+之后 Mutex阻塞是可抢占的
	// 阻塞的 main goroutine 会让出 P, 触发调度器寻找可执行的 G
	lock.Lock()
	lock.Unlock()
	println("mutex unlocked.")
	
	println("===============>")
	// 默认情况下 GOMAXPROCS 和 CPU 数量保持一致
	fmt.Printf("GOMAXPROCS: %d\n", runtime.GOMAXPROCS(0))
	fmt.Printf("Goroutine: %d\n", runtime.NumGoroutine())
	fmt.Printf("CPU: %d\n", runtime.NumCPU())
	fmt.Printf("OS: %s\n", runtime.GOOS)
	fmt.Printf("Arch: %s\n", runtime.GOARCH)
	fmt.Printf("Version: %s\n", runtime.Version())
	fmt.Printf("Compiler: %s\n", runtime.Compiler)

	println("===============>")
	// 终止程序
	
	// 之前q已经被close了
	q = make(chan struct{})
	go func() {
		defer close(q)
		defer println("done.")

		a()
		b()
		c()
	}()

	<-q

	println("===============>")
	// 在 main goroutine 中调用 Goexit() 它会等待其他任务结束，然后崩溃进程

	q = make(chan struct{})
	go func() {
		defer close(q)
		defer println("done.")
		time.Sleep(time.Second)
	}()
	
	// 在 main goroutine 中调用 Goexit，其会等待其他任务结束，然后崩溃进程。
	// main.mian()会立刻结束
	// main goroutine 会被销毁
	// defer会正常执行
	// 程序不会立即退出
	// 如果有其他goroutine正在运行，那就等它们运行完了再崩溃
	defer println("main done.")
	// 报错的信息是 runtime.Goexit deadlock
	// 原因: 销毁main goroutine 后程序不会立即结束，而是runtime会进入调度循环
	// 发现没有goroutine可以执行
	// 而且程序没有走 main return 的正常退出路径
	// runtime 判定 all goroutines - deadlock
	// runtime.Goexit()
	<-q

	println("===============>")

	// os.Exit
	go func() {
		defer println("done.")
		time.Sleep(time.Second)
	}()

	defer println("main done.")
	// os.Exit 可在任意位置结束进程。不等待其他任务，也不执行延迟调用defer
	// os.Exit(0)

	println("===============>")
	// go 中 所有 协程 共享同一个虚拟地址空间
	var gls [2]struct{
		id int
		ret int
	}

	var swg sync.WaitGroup
	swg.Add(len(gls))

	for i := 0; i < len(gls); i++ {
		go func(id int) {
			defer swg.Done()

			gls[id].id = id
			gls[id].ret = (id + 1) * 100
		}(i)
	}
	swg.Wait()
	fmt.Printf("%+v\n", gls)
}

func aysn() {
	// sync
	quit := make(chan struct{})
	data := make(chan int)

	go func() {
		data <- 11
	}()

	go func() {
		defer close(quit)

		fmt.Printf("data: %d", <-data)
		fmt.Printf("data: %d\n", <-data)
	}()

	data <- 22
	<-quit
	// 22 11 和 11 22 都有可能，只不过 11 22 很少很少
	// 因为当前环境下调度几乎总是 main 先跑

}
// 为了避免重复关闭
func closechan[T any](c chan T) {
	defer func() {
		recover()
	}()
	close(c)
}

func async() {
	println("===============>")
	// async
	quit := make(chan struct{})
	data := make(chan int, 3)

	data <- 11
	data <- 22
	data <- 33

	println(cap(data), len(data))

	go func() {
		defer close(quit)

		println(<- data)
		println(<- data)
		println(<- data)
		
		println(<- data)
	}()
	// 缓冲区已满，阻塞
	data <- 44
	// 阻塞，直到 gorouine 执行defer
	<- quit

	var a, b chan int = make(chan int, 3), make(chan int)
	var c chan bool

	println(a == b)
	println(c == nil)
	println(a, unsafe.Sizeof(a))

	closechan(a)
	closechan(a)
	closechan(b)
	closechan(b)
}

type Queue[T any] struct {
	sync.Mutex

	ch chan T
	cap int
	closed bool
}

func NewQueue[T any](cap int) *Queue[T] {
	return &Queue[T] {
		ch: make(chan T, cap),
	}
}

func (q *Queue[T]) Close() {
	q.Lock()
	defer q.Unlock()
	if !q.closed {
		close(q.ch)
		q.closed = true
	}
}

func (q *Queue[T]) IsClosed() bool {
	q.Lock()
	defer q.Unlock()
	return q.closed
}

func queueTest() {
	println("===============>")
	var wg sync.WaitGroup
	q := NewQueue[int](3)

	for i := 0; i < 10; i++ {
		wg.Add(1)

		go func() {
			defer wg.Done()
			defer q.Close()
			println(q.IsClosed())
		}()
	}
	wg.Wait()

	// 从 nil 通道接收 直接 panic
	// chan receive (nil chan)
	// <- (chan struct{})(nil)
}

func channelTest() {
	println("===============>")
	var wg sync.WaitGroup
	wg.Add(2)

	c := make(chan int)
	var send chan<- int = c
	var recv <-chan int = c

	// recv
	go func() {
		defer wg.Done()
		for x := range recv {
			println(x)
		}
	}()

	// send
	go func() {
		defer wg.Done()
		defer close(c)
		for i := 0; i < 3; i++ {
			send <- i
		}
	}()

	wg.Wait()


	exit := make(chan struct{})

	chans := make([]chan int, 0)
	chans = append(chans, make(chan int))
	chans = append(chans, make(chan int))

	// select 语句是 编译时固定的
	// 不能写成 select { case <- chans[i]: } i 是变量

	go func() {
		defer close(exit)
		// reflect.Select 是 Go 反射包提供的动态select
		// 处理运行时构建的case列表
		// 普通 select 是编译时固定的，不能动态添加/删除case
		// 这个 cases 数组子啊曾哥goroutine生命周期中只构建一次
		cases := make([]reflect.SelectCase, len(chans))
		for i, c := range chans {
			cases[i] = reflect.SelectCase{
				// 操作方向: 发送、接收、还是默认
				Dir: reflect.SelectRecv,
				// channel 的反射值,必须是channel类型
				// 把普通的 channel值 c 类型转换成 反射值(reflect.Value类型)
				// 该反射值内部持有对原channel的引用，不会复制channel
				// 并且这个反射值会记住它是哪个具体的channel，后面即使将chans[i]改成别的值，
				Chan: reflect.ValueOf(c),
				// 如果是发送方向，这里放要发送的值
				// Send: reflect.ValueOf(v)
			}
		}
		// 上面等价于
		// select {
		//	case v, ok := <- chans[0]:
		//
		//	case v, ok := <- chans[1]:
		//
		//}
		for {
			// index 哪个case被选中
			// value 接收到的值
			// ok 是否成功接收
			index, value, ok := reflect.Select(cases)
			
			if !ok {
				// 这里 cases[index].Chan 依然指向原来的已关闭channel
				// Golang中的 reflect.Select中 如果 case 的 Chan 是已关闭的正常channel->该case永远可能立即选中,返回 ok = false, value = 零值
				// 如果是 nil channel -> 该 case 永不选中
				chans[index] = nil
				n := 0
				for _, c := range chans {
					if c == nil {
						n++
					}
					if n == len(chans) {
						return
					}
					continue
				}
			}
			println(index, value.Int(), ok)
		}
	}()

	chans[0] <- 1
	chans[1] <- 2

	for _, c := range chans {
		// 已关闭的channel 总是 "就绪"
		close(c)
	}

	<-exit
}

func newRecv[T any](cap int) (data chan T, done chan struct{}) {
	data = make(chan T, cap)
	done = make(chan struct{})

	go func() {
		defer close(done)
		// 不断从data接收值，直到data被关闭
		for v := range data {
			println(v)
		}
	}()
	return data, done
}

func recvTest() {
	println("===============>")
	
	data, done := newRecv[int](3)

	for i := 0; i < 10; i++ {
		data <- i
	}

	close(data)
	<-done
}

func main() {
	var once sync.Once

	f1 := func() {
		println("f1")
	}
	
	f2 := func() {
		println("f2")
	}

	once.Do(f1)
	once.Do(f1)
	once.Do(f2)
	once.Do(f2)
}

