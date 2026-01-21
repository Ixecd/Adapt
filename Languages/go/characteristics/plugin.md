## Plugin
简单的动态库（.so）装载和查找符号

eg.

```text
	test/
	|
	+-- main.go
	|
	+-- mylib.go
			|
			+-- add.go
```

```go
// mylib/add.go

package mylib

func Add(x, y int) int {
	return x + y
}
```

```go
// main.go

package main

import (
	"test/mylib"
)

func main() {
	println(mylib.Add(1, 2))
}
```

将 `mylib` 子包改成插件（plugin）模式

- 将`mylib`初始化为独立模块，从`test`里排除
- 将`package mylib`改为`package main`
- 添加一些导出和非导出成员，用于测试
- 以`-buildmode=plugin`方式编译

```go
// add.go

package main

var X = 100
const S = "abc"

func init() {
	println("plugin init.")
}

func hello() {
	println("hello, world!")
}

func Add(x, y int) int {
	return x + y
}

func main() {
	println("plugin.main.")
}
```

```bash
cd mylib
go mod init mylib
go build -buildmode=plugin
# nm: 列出目标文件中的符号表，包括函数、变量等
nm mylib.so | grep mylib
```

接下来就可以通过修改`main.go`，动态加载和调用

```go
package main

import (
	"plugin"
	"log"
)

func test() {
	p, err := plugin.Open("./mylib/mylib.so")
	if err != nil {
		log.Fatalln(err)
	}

	// Add -------------------------------

	s, err := p.Lookup("Add")
	if err != nil {
		log.Fatalln(err)
	}

	add, ok := s.(func(int, int) int)
	if ok { println(add(1, 2)) }

	// X ---------------------------------
	s, err = p.Lookup("X")
	if err != nil {
		log.Fatalln(err)
	}

	x, ok := s.(int)
	if ok { println(x) }

	// hello -----------------------------

	s, err = p.Lookup("hello")
	if err != nil {
		log.Fatalln(err)
	}
}

func main() {
	test()
}
```

```bash
go build -o test && ./test
```

- 初始化函数（init）正常执行，且执行一次
- 入口函数（main）不会执行
- 非导出成员不可用