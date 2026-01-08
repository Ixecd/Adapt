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

### 并行
默认情况下，包内串行，多包并行

### 内部实现
每个测试函数都在独立`goroutine`内运行，正常情况下，执行器会阻塞，等待`test goroutine`完成

### 助手
将调用`Helper`的函数标记为测试助手

输出测试信息时跳过助手函数，直接显示测试函数文件名、行号

- 直接在测试函数中调用无效
- 测试助手可用作断言


### 清理
为测试函数注册清理函数，在测试结束时执行

- 如注册多个，则按FILO顺序执行
- 即便发生`panic`，也能确保清理函数执行

```go
func TestA(t *testing.T) {
	t.Cleanup(func(){ println("cleanup") })
	t.Cleanup(func(){ println("cleanup2") })
	t.Cleanup(func(){ println("cleanup3") })

	t.Log("body.")
}
```
和 `defer` 的区别: 即便在其他函数内注册，也会等测试结束后再执行

可用来写`Helper`函数，意思就是**谁申请，谁负责定义销毁规则**
```go
func newDatabase(t *testing.T) *DataBase{
	t.Helper()

	d := Database.Open()
	// t.Cleanup 的声明周期不是绑定到当前函数作用域
	// 而是绑定到 testing.T 对象的声明周期上
	t.Cleanup(func() {
		d.Close()
	})

	return &d
}

// 也就是说，当TestDB结束的时候，才会调用newDatabase的Cleanup函数
func TestDB(t *testing.T) {
	db := newDatabase(t)
	...
}
```

## 子测试
将测试函数拆分为子测试，更符合套件(suite)模式

- 便于编写初始化(setup)和清理(teardown)代码
- 表驱动(table-driven)时，拆分成多个并发测试
- 便于观察子测试时间，不用考虑外部环境影响

```go
func TestA(t *testing.T) {
	time.Sleep(1 * time.Second)
}

func TestB(t *testing.T) {
	time.Sleep(2 * time.Second)
}

func TestC(t *testing.T) {
	time.Sleep(3 * time.Second)
}

func TestSuite(t *testing.T) {
	t.Log("setup")
	defer t.Log("teardown")

	t.Run("TestA", TestA)
	t.Run("TestB", TestB)
	t.Run("TestC", TestC)
}
```

直接测试Suite `go test -v -run "Suite"`

按名称单独执行子测试

`go test -v -run "Suite/[AB]"`
`go test -v -run "Suite/B"`

支持子测试并行
```go
func TestSuite(t *testing.T) {
	tests := []int{2, 3}
	
	// 主测试结束前，打印teardown，后执行所有子测试。这样其实不对，因为预期的是所有测试都完了之后再teardown的
	// 解决办法：在 for 外面再套一层 Run 调用
	defer t.Log("teardown")
	
	// t.Run("group", func(t *testing.T){

	for v := range tests {
		// 避开闭包延迟
		x := v
		// t.Run里的匿名函数是一个闭包，当所有子测试被唤醒时，循环可能已经跑完了，v已经变了
		t.Run(fmt.Sprintf("Test%d", x), func(t *testing.T) {
			// 告诉主测试函数，把自己挂起，先执行后面的，最后主测试函数准备退出时，将所有挂起的函数都唤醒。
			t.Parallel()
			time.Sleep(time.Duration(x) * time.Second)
			println(x)
		})
	}

	//})
}
```

### 表驱动
表驱动(table-driven)将数据和逻辑分离，便于维护和扩展

- 子测试，确保所有数据被测试
- 并行子测试，提高效率

- 建议用相同命名，规范化
- 使用短命`want/got`更好
- 输出信息应该面向自然阅读

```go
func add(x, y int) int {
	return x + y
}

func TestAdd(t *testing.T) {

	// 数据表
	var tests = []struct {
		x		int
		y		int
		want	int
	}{
		{1, 2, 3},
		{2, 3, 5},
		{3, 4, 7},
	}


	// 测试
	for _, tt := range tests {
		// 规避闭包延迟
		o := tt

		t.Run("", func(t *testing.T) {
			t.Parallel()

			got := add(o.x, o.y)
			if got != o.want {
				t.Errorf("add(%d, %d): want %d, got %d", o.x, o.y, o.want, got)
			}
		})
	}
}
```

### 覆盖率

代码覆盖率(code coverage)是度量测试完整和有效性的一种手段

- 通过覆盖率值，分析测试代码编写质量
- 检测是否提供完备测试条件，是否执行了全部目标代码
- 量化测试，让白盒测试真正起到应有的质量保障作用

`go test -cover`


### 示例
示例代码最大用途不是测试，而是导入到GoDoc等工具生成的帮助文档

比对输出(stdout)结果和内部output注释是否一致来判断是否成功

- 不能使用内置函数 print/println，因为它们输出到 `stderr`
- 没有输出注释的示例被编译，但不执行

```go
func ExampleAdd() {
	fmt.Println(add(1, 2))
	fmt.Println(add(2, 3))
}
```

`go test -v -run "Example"`

### 入口函数

像`main.main`那样，为测试提供一个入口函数

- 同样放在 `_test.go` 文件中
- 为整个测试过程提供 `setup/teardown` 机制
- 在 main goroutine 中执行

```go
// 解决的是包级别的问题
// 全局共享资源、控制全局状态、运行在main goroutine
// 如果写了这个函数，那么由它来控制测试的启动、环境陪孩子和最终清理
func TestMain(m *testing.M) {

	// setup
	// 调用测试函数
	code := m.Run()

	// teardown
	// 不会执行 defer
	os.Exit(code)
}
```

