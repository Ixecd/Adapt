## embed
将文件嵌入到二进制可执行文件内

```go
package main

import (
	_ "embed"	// 必须导入
)

//go:embed main.go
var main_go string

//go:embed main_test.go
var test_go []byte

func main() {

}
```

如果使用 `embed.FS`，可嵌入更多文件（包目录及其子目录）

```go
package main

import (
	"embed"
	"fmt"
)

//go:embed mylib/* mylib2/*
//go:embed *.go
var dir embed.FS

func main() {
	fmt.Println(dir.ReadFile("main.go"))
	fmt.Println(dir.ReadFile("mylib2/add.go"))
}
```
- 在`//go:embed`指令和变量之间只能有空行或注释
- 变量必须是: `string`，`[]byte`，`embed.FS`

- 路径不能使用 `./` `../`，开头和结尾不能用`/`
- 用引号包含有空格的路径名（"a b c.txt"）

- 只有 `embed.FS` 允许多个文件
- 可以有多个 `//go:embed` 指令
- 单指令以空格分隔多个路径 （`//go:embed mylib/* mylib2/*`）
- 以 `*` 表示多文件模糊匹配