## 测试
**测试驱动开发(Test-Driven Development, TDD)** 是一种软件开发过程中的应用方法

- **实现**: 先写测试，然后编码快速实现
- **重构**: 在测试保护下，去除冗余代码，提高代码质量

**类别**
- 单元测试(unit testing): 对程序模块进行正确性检验。
- 基准测试(benchmark testing): 对某项性能指标进行测量和评估。
- 模糊测试(fuzz testing): 输入随机数据，发现潜在错误。

- 黑盒测试(black box): 无视内部构造，测试外部接口是否符合功能设计。
- 白盒测试(white box): 深入内部构造，验证内部逻辑是否符合设计规格。

## 单元测试
为测试非导出成员，测试文件也放在目标包内

- 测试文件以 `_test.go` 结尾
	- 通常与测试目标主文件名相同，如`sort_test.go`
	- 构建命令(`go build`)忽略测试文件

- 测试命令(`go test`)
	- 忽略以 `_` 或 `.` 开头的文件
	- 忽略 `testdata` 子目录
	- 执行 `go vet` 检查

- 测试函数名`Test<Name>`
	- `Test` 为识别标记
	- `<Name>` 为测试名称，首字母大写。比如`TestSort`

- 测试函数内以 `Error`、 `Fail` 等方法标记失败
	- `Fail`: 失败，继续当前函数
	- `FailNow`: 失败，终止当前函数
	- `SkipNow`: 跳过，终止当前函数
	- `Log`: 输出信息，仅失败或`-v`时有效
	- `Error`: Fail + Log
	- `Fatal`: FailNow + Log
	- `Skip`: SkipNow + Log

	- `os.Exit`: 失败，测试进程终止

### 模式
- **本地模式** : `go test`, `go test -v` 
	- 不缓存测试结果

- **列表模式** : `go test math`, `go test. `, `go test ./...`
	- 缓存结果，直接输出
	- 缓存输出有`cached`标记
	- 执行 `go clean -testcache` 清除缓存

### 执行

- `go test` 			测试当前包
- `go test math` 		测试math包
- `go test ./mylib` 	使用相对路径测试mylib包
- `go test ./...` 		测试当前目录下的所有包

