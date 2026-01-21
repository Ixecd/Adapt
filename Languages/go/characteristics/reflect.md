## reflect
反射可在运行期「动态」获取类型（type）和值（value）信息

- 代码繁琐，可读性差
- 性能较差，不及unsafe指针

```go
func TypeOf(i any) Type
func ValueOf(i any) Value
```

类型（Type）表示具体的静态类型，而类别（Kind）则表示其底层结构。相比Type，Value更倾向于对值的处理。两者有许多同名方法，但返回值可能不同

```go
package main

import (
	. "reflect"
)

func main() {
	type X int
	var x X = 1

	v := ValueOf(x)
	t := v.Type()

	println(t.PkgPath() + "." + t.Name())

	switch v.Kind() {
		case Int: println(v.Int())
		case String: println(v.String())
	}

	var y int = 2
	ty := TypeOf(y)

	println(t ==ty)				// false
	println(t.Kind() == ty.Kind())	// true
}
```

如果是`nil`接口，那么无法反射。因为接口必须和实现类型结合，才能有完整 itab 信息

- IsValid: 验证Value自身是否为零值
- IsZero、IsNil: 验证目标对象