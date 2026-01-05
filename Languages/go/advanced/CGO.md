## CGO
在 Go 和 C 之间相互调用，但是注意***CGO is not GO**，除非必要，不要使用

- 编译慢，调试麻烦
- 不支持交叉编译
- 存在性能问题，内存泄露
- 需要在C和Go栈间切换
- 可能导致线程数量激增

- 受 `CGO_ENABLED` 环境变量影响，默认关闭

直接以注释方式将C代码嵌入到Go源文件中

```go
package main
// #include <stdio.h>
// void print_hello() {
//     printf("Hello, World!\n");
// }
import "C"

func main() {
	C.print_hello()
}

```
### 独立文件
单独保存在`.c`文件中，以`GCC/GDB`单独调试
等C代码无误后，在Go内 `#include` 头文件，编译器会自动编译（链接）`.c`, `.s`, `.go`文件

### 动态链接
也可以将`.c`编译成动态库,链接到Go程序
```bash
mkdir ./lib
mv hello.* ./lib
cd ./lib
gcc -g -O0 -fPIC -shared -o libhello.so hello.c
```

```go
package main

/*

	#cgo CFLAGS: -I${SRCDIR}/lib
	#cgo LDFLAGS: -L${SRCDIR}/lib -lhello -Wl, -rpath=.:./lib
	#include "hello.h"

*/
import "C"

func main() {
	C.print_hello()
}
```

### 静态链接
将libc链接到可执行文件内
```bash
go build -ldflags '-extldflags "-static"'
```