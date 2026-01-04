package main

import (
	"fmt"
	"unsafe"
	"package/test/mylib"
)

// 包内的任意的go文件内都可以定义一到多个 init 初始化函数
// 初始化函数由编译器生成代码自动执行(仅执行一次)，不能被其他代码调用


// 所有初始化函数被编译器整合到一个特殊数据结构内
// 在程序启动初始化阶段，在同一 goroutine 内依次执行

// 代码重构时，将一些内部模块陆续分离出来，以独立包形式维护。
// 此时，首字母大小写控制就过于粗。
// 我们希望其 导出成员 仅限特定范围内访问，而不是向所有用户公开
// 所以有了 内部包(internal package)

// 内部包(含自身)只能被其父目录(含所有层次子目录)访问
// 内部包私有成员,依然只能在自己包内访问
func main() {
	l := mylib.NewData()
	// l.b undefined
	// println(l.a)

	// 强行将l转换成一个我自己新定义的结构体指针，从而访问/修改它的内存
	d := (*struct{
		x int
		y int
	})(unsafe.Pointer(l))

	d.y = 100

	fmt.Println(d.y)

	println("=====================>")

	t := mylib.DataTmp{}
	t.Setb(100)
	t.Test()
}