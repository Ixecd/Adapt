## Container
数据结构有很多种，标准库只提供了常用的链表和最小堆
如对性能或存储量有较高要求，建议使用第三方专业版本

## 双向链表
平平无奇的双向链表😂

```go
type Element struct {
	Value any
}
```

## 环状链表
由多个 Ring 元素构成一个环状链表。持有任何一个都可以访问全部

```go
type Ring struct {
	Value any
}
```

```go
package main

import (
	"container/ring"
	"fmt"
)

func main() {
	r := ring.New(3)
	
	for i := 0; i < r.Len(); i++ {
		r.Value = i + 100
		r = r.Next()
	}

	// 合并
	r2 := ring.New(2)
	r2.Value = 4
	r2.Next().Value = 5
	r.Link(r2)

	// 遍历
	r.Do(func(v any) {
		fmt.Println(v.(int))
	})

	r3 := r.Unlink(2)

	r3.Do(func(v any) {
		fmt.Println(" ", v.(int))
	})
}
```

## 最小堆
最小堆结构，常用来实现优先级队列
必须实现`heap.Interface`接口

```go
// sort
type Interface interface {
	Len() int
	Less(i, j int) bool
	Swap(i, j int)
}

// heap
type Interface interface {
	sort.Interface

	Push(x any)
	Pop() any
}
```

实现上面的方法之后，通过 `heap` 里的函数进行操作，这里 「有序」 是针对 `heap` 而言的 

- `Init`: 将目标初始化为有序堆
- `Push`: 压入数据，并保持有序
- `Pop`: 弹出 「最小」 数据
- `Fix`: 修改元素值后修复，是使得堆保持有序状态
- `Remove`: 删除元素，并维持堆有序

## encoding
常用编码转换器

- `base64`: 基于64个可打印字符来表示二进制数据
- `binary`: 抓换数字和字节数组
- `csv`: 读写csv表格文件
- `gob`: 二进制序列化
- `hex`: 十六进制编码
- `json`: JSON编码
- `pem`: TLS密钥和证书编码
- `xml`: XML编码

## base64
使用 `NewEncoder` 记得调用 `Close` 方法

## binary
字节存储顺序(byte order)和处理器架构有关，Intel x86 是小端序

- 小端: 三低
- 大端: 高位字节放在低端地址

## csv
逗号分隔值(Comma-Separated Values)，以纯文本格式存储表格数据(数字和文本)

- 纯文本，使用某个字符集。比如 ASCII、Unicode
- 由记录组成
- 每条记录被分隔符分隔为字段
- 每条记录都有同样的字段序列

**双引号**
用双引号表示包含空格的字段内容，其中可包含换行和分隔符。两个连续双引号进行转义

**空白**
空格等属于字段内容的组成部分，空行被忽略，但由空白符填充的行不是空行

## json
- 递归对象字段结构，必须是导出成员
- 自动调用`MarshalJSON`、`MarshalText`接口方法
- 支持指针

解码目标除结构体外，也可以是`map`

- 忽略不存在的字段
- 优先精准匹配字段名。找不到时，忽略大小写
- 字段顺序不影响

- 可为匿名字段添加`tag`标记
- 字段名和匿名字段冲突，当前字段优先
- 相同层次的多匿名字段冲突，忽略冲突字段

