## 导入

- 导入的是**完整的模块路径**，不是包名，也不是本地路径
- 编译器以此搜索标准库、项目根目录，以及缓存目录
- 引用包成员，用包名(package)，而非导入路径

默认为 module 模式，第三方依赖依然存放在 GOPATH 目录下，vendor依然支持，但仅用于构建依赖

```go
import "net/http"		// $GOROOT/src/net/http
import "github.com/xxx"	// $GOMODCACHE/github.com/xxx
```

使用别名来解决同名冲突问题
```go
import (
	osx	"osx/lib"
	nix	"linux/lib"
)
```
不同导入方式，及其成员引用
```go
import (
		"math"	// 默认 math.Sin
	m	"math"	// 别名 m.Sin
	.	"math"	// 简名 Sin，常用于单元测试中
	_	"math"	// 初始化:无法引用，仅用来初始化目标包
)
```

## 模块
模块（module）是包（package）和其依赖项（dependency）的集合

直接的体现是 `go.mod` 文件，存储了模块路径、编译器版本，以及依赖项列表

模块是依赖管理方式，是发布和版本控制单元

**module = go.mod + package + subs...**

- 模块路径(module): 在 `go.mod` 中声明的名称标识符
- 根目录(module root directory): 包含 `go.mod` 文件的目录
- 主模块(main module):执行`go`命令时，所在目录对应的模块

### 初始化
模块对应一个包含 `go.mod` 的源码目录，所有不含go.mod的子目录都是其成员

如果子目录包含 `go.mod`，那么它将是独立模块，不再属于当前模块

### 依赖管理
命令 `go get` 添加、下载（更新）依赖性，其他操作可用 `go mod` 完成

- 从`GOPROXY`下载源码到本地缓存`GOMODCACHE`目录
- 向`go.mod`、`go.sum`添加依赖项和验证信息
- 使用 `go clean -modcache` 清除缓存

```bash
go get . 							# 分析源码，添加所有依赖
go get example.com/my				# 指定模块，下载最新版本

go get example.com/my@v1.3.4		# 指定版本号
go get example.com/my@latest		# 最新版本

go get example.com/my@3d7f32		# 伪版本号(commit hash)
go get example.com/my@bufix			# 分支(branch)
```

使用 `go mod` 管理模块
- init: 初始化模块，创建 `go.mod` 文件
- tidy: 添加依赖，移除不需要的依赖项
- edit: 命令行方式编辑模块设置
	- -fmt: 格式化 go.mod 文件
	- -module: 模块路径
	- -go: 编译器版本
	- -retract, -dropretract: 标识有问题需要忽略的版本

### 版本标识
版本标识模块不可变快照
- 以 `v` 开头，然后是语义版本(semantic version)

- 不兼容更新，递增主要版本号
- 添加功能的兼容更新，递增次要版本号
- 优化和修复缺陷，不影响导出接口，递增补丁版本号
- 正式发布前的预发行版本，添加 -pre 后缀

- 使用`git tag`之类的功能标记语义化版本号
- 如果没有标记，则使用提交(commit)信息(time,ident)构成伪版本号

**v(major).(minor).(pathc)-(pre|beta)**

如果新版本和旧版本有相同导入路径，则必须保证向后兼容

**主版本号出现在模块路径尾部，表示重大不兼容更新**，比如etcdctl/v3，import时必须用包含版本号的全路径

v0和v1不需要使用版本后缀

v0本身就表示不稳定且不具有兼容性保证，v1为默认版本

有预发行后缀也表示该版本不稳定，不受兼容性约束

### 工作空间
以工作空间(workspace)解决多模块开发遇到的问题

- 编译时，找不到未发布模块(非子包)
- 只能以 replace 将模块路径替换为本地路径
- 污染的 go.mod 意外提交到代码仓库

```bash
go work init								# 初始化工作空间
go work use ./module1 ./module2 ./module3	# 使用模块
go work user ../workspace					# 使用工作空间
```
or
```go
// go.work
go 1.24

use (
	./module1
	./module2
	./module3
	../workspace
)
```
import到workspace内的其他模块，无需 require 指令。如果必须添加 require，编译时可关闭 GOPROXY
```bash
GOPROXY=off go build
```

### 环境变量
- GO111MODULE: 模块模式，off/on/auto
- GOMODCACHE: 模块缓存目录
- GOPROXY: 模块代理地址
- GOSUMDB: 模块数据安全校验数据库
- GOPRIVATE: 私有模块地址，绕过GOPROXY直接获取，以逗号分隔
GOPRIVATE是GONOPROXY、GONOSUMDB的默认值

### 离线编译
打包含依赖在内的所有源码，进行离线编译

```bash
go mod vendor								# 将依赖复制到./vendor目录下
GOWORK=off go build -mod vendor				# 禁用工作空间，以vendor方式编译
```

-mod可选的值都有 readonly、vendor、mod、patch
- readonly: 编译器以只读模式运行，如果go.mod需要更新，就会直接报错并失败，适合用于CI
- vendor: 编译器以vendor模式运行，将依赖复制到./vendor目录下，适合离线编译
- mod: 编译器以mod模式运行，使用go.mod中的依赖，适合开发阶段
- patch: 编译器以patch模式运行，仅允许更新到补丁版本

工作空间支持vendor目录