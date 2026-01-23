## Bootstrap（引导）

随意编译一个可执行程序

```bash
gdb test
```

也可以用 readelf 获取起始地址

```bash
readelf -h ./test

addr2line -e ./test -a 0x401000
```

查看 go/src/runtime 目录下以汇编实现的引导过程源码

核心流程是创建 main goroutine（runtime.main），并执行mstart进入调度循环

## init

在 rt0_go 内，先完成环境初始化，然后再启动运行时

- runtime·args
- runtime·osinit
- runtime·schedinit
- runtime·main

### 系统

系统环境相关的主要是逻辑CPU核数量，以及HugePage页大小

标准库 debug 返回的核数量就是该变量

### 调度器

调度器执行前，需要对内存分配器、垃圾回收器等一些列部件进行初始化

### 运行时

运行时入口 runtime.main 函数。由此，程序算是正是「执行」

### 初始化函数

初始化函数分成两部分:

- 运行时
- 用户代码

所有初始化函数被收集存储到 initTask 结构内

### 退出进程

当用户入口函数（main.main）执行结束后，调用exit结束进程，返回状态码

不会等待其他goroutine结束
