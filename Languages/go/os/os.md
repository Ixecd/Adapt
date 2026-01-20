## OS

### exec

使用 `os.Process` 可以创建进程以执行其他程序，不过常用的是其封装版本

#### cmd

创建 `Cmd`，然后选择相关执行方法

- Start + Wait
- Run = Start + Wait
- Output = Run + Stdout
- CombinedOutput = Run + (Stdout + Stderr)

基本输入输出，最直接的做法是 pipe 方法

- StdinPipe
- StdoutPipe
- StderrPipe

当然，也可以直接设置字段
```go
type Cmd struct {
	Stdin io.Reader
	Stdout io.Writer
	Stderr io.Writer
}
```

需要注意，Wait方法会清理进程资源。所以获取输出结果，必须在此之前

```go
package main

import (
	"fmt"
	"io"
	"log"
	"os/exec"
)

func main() {
	cmd := exec.Command("ls", "-l", "/usr/local/go/src")
	out, _ := cmd.StdoutPipe()

	if err := cmd.Start(); err != nil {
		log.Fatalln(err)
	}

	b, _ := io.ReadAll(out)
	fmt.Println(string(b))

	cmd.Wait()
	fmt.Println(cmd.ProcessState)
}

func main() {
	out, _ := exec.Command("ls", "-l", "/usr/local/go/src").Output()
	fmt.Println(string(out))
}

func main() {
	cmd := exec.Command("ls", "-l", "/usr/local/go/src")
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	if err := cmd.Run(); err != nil {
		log.Fatalln(err)
	}
	fmt.Println(cmd.ProcessState)
}
```

#### 僵尸进程

主进程必须或间接调用 Wait 获取子进程的退出状态，否则会导致僵尸（zombie）进程。除非主进程先终止，由系统init进程完成状态检查工作

```go
package main

import (
	"fmt"
	"os/exec"
)

func main() {
	cmd := exec.Command("ls", "-lh", "/usr/local/go/bin")
	cmd.Start()

	fmt.Println(cmd.Process.Pid)
	fmt.Scanln()
}
```

#### 传递信息

和 fork 调用不同，默认会执行 exec 操作，重置相关信息。因此，要向子进程传递信息，必须借助 Env 和 ExtraFiles 属性

```go
package main

import (
	"fmt"
	"io"
	"log"
	"os"
	"os/exec"
	"strconv"
)

func parent() {
	// 通过 Args 区分父子进程
	cmd := exec.Command(os.Args[0], "-child")
	cmd.Stdout = os.Stdout

	// 文件总数
	files := []string{"main.go", "main_test.go"}
	cmd.Env = append(cmd.Env, fmt.Sprintf("count=%d", len(files)))

	for i := 0; i < len(); i++ {
		name := files[i]

		file, _ := os.Open(name)
		defer file.Close()

		cmd.Env = append(cmd.Env, fmt.Sprintf("%d=%s", 3+i, name))
		cmd.ExtraFiles = append(cmd.ExtraFiles, file)
	}

	if err := cmd.Run(); err != nil {
		log.Fatalln(err)
	}
}

func child() {
	// 获取文件总数
	count, _ := strconv.Atoi(os.Getenv("count"))

	// 从3+i开始，读取文件名和内容
	for i := 0; i < count; i++ {
		name := os.Getenv(strconv.Itoa(3+i))

		file := os.NewFile(uintptr(3+i), name)
		defer file.Close()

		b, _ := io.ReadAll(file)
		fmt.Println(name, len(b))
	}
}

func main() {
	if len(os.Args) > 1 {
		fmt.Println("child:", os.Getpid(), os.Getppid())
		child()
	}

	fmt.Println("parent:", os.Getpid())
	parent()
}
```

### Process

很少直接 Process 创建子进程，而是 FindProcess 获取某个已运行进程。进而向其发送信号，或终止

- Kill: 发送 SIGKILL 强制终止信号
- Signal: 发送信号
- Release: 释放进程资源
- Wait: 等待进程终止，释放资源，返回相关状态信息

```go
package main

import (
	"fmt"
	"log"
	"os"
	"os/exec"
	"strconv"
	"strings"
)

func main() {
	log.SetFlags(log.Lshortfile)

	b, err := exec.Command("pidof", "top").CombinedOutput()
	if err != nil { log.Fatalln(err) }

	pid, _ := strconv.Atoi(strings.TrimSpace(string(b)))
	p, err := os.FindProcess(pid)
	if err != nil { log.Fatalln(err) }

	p.Kill()
	fmt.Println(p.Wait())
}
```

### signal

信号时软中断（software interrupt），提供一种处理异步事件机制

事件可能来自系统外部（用户中断），也可能来自程序内部或内核。作为进程间通信（IPC）的基础构成，可向其他进程发送信号

针对信号的操作包括：忽略和捕获。如果不做处理，则执行默认操作

- SIGHUP: 控制终端关闭
- SIGINT: 用户产生中断字符（Ctrl+C）
- SIGQUIT: 用户产生退出字符（Ctrl+\）
- SIGTERM: 进程终止，通常是 kill 操作

大部分信号定义在 syscall 包，os包中仅有两个

```go
// os

var (
	Interrupt Signal = syscall.SIGINT
	Kill      Signal = syscall.SIGKILL
)
```

**默认情况下**:

- SIGINT、SIGTERM、SIGHUP 导致进程终止
- SIGQUIT 终止进程前生成 core dump
- SIGKILL （kill -9）和 SIGSTOP 不能被忽略或捕获

改变信号默认行为方式，用 Notify 函数注册信号通道

```go
package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"
)

func main() {
	sig := make(chan os.Signal)
	signal.Notify(sig, syscall.SIGINT, syscall.SIGQUIT, syscall.SIGTERM)

	go func() {
		for s := range sig {
			fmt.Println(s)
		}
	}()

	fmt.Println(os.Getpid())
	select {}
}
```

- Ignore: 忽略信号的默认行为
- Rest: 按Signal恢复默认行为
- Stop: 按Notify chan恢复默认行为

```go
func main() {
	// 忽略 Ctrl+C
	signal.Ignore(syscall.SIGINT)
	fmt.Scanln()
}

func main() {
	sig1 := make(chan os.Signal)
	sig2 := make(chan os.Signal)

	// 将指定的系统信号转发到通道
	// 同一个信号可以被转发到多个通道
	signal.Notify(sig1, syscall.SIGINT)
	signal.Notify(sig2, syscall.SIGQUIT)

	for {
		select {
			case s := <- sig1: fmt.Println(s)
			case s := <- sig2: signal.Stop(sig1); fmt.Println(s)
		}
	}
}
```

除了退出信号外，还常用 SIGUSR1、SIGUSR2 来实现通知事件，比如重载配置文件

```go
package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"
)

func main() {
	sig := make(chan os.Signal)
	signal.Notify(sig, syscall.SIGINT, syscall.SIGUSR1, syscall.SIGUSR2)

	go func() {
		for s := range sig {
			switch s {
				case syscall.SIGINT:
				os.Exit(1)
				case syscall.SIGUSR1, syscall.SIGUSR2:
				fmt.Println("reload config")
			}
		}
	}()

	fmt.Println(os.Getpid())
	select{}
}
```