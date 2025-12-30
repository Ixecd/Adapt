package main

import (
	_ "os"
	"fmt"
	"sync"
	"time"
	"context"
	"runtime"
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

func main() {
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

	println("===============>")
	// async
}