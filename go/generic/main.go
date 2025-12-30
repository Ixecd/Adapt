package main

import "fmt"

// GCShape + Dictionary

// GCShape描述了一个类型在Go运行时(尤其是垃圾回收期和内存分配器)眼中的形状
// 主要包括 类型的大小、对齐要求、类型中哪些部分包含指针
// 在编译期间，Go编译器会根据类型结构生成唯一的GCShape标识符
// 相同GCShape的类型，对GC来说处理方式完全相同
// 不需要为每个命名类型生成新函数
// 大幅减少代码膨胀
// 对于指针/接口类型，共享代码后无法静态知道具体方法，需要运行时动态分发

// 当多个具体类型共享同一个GCShape时，但行为不同时，编译器会生成一个Dictionary


// 任意指针类型、或具有相同底层类型(underlying type)的类型
// 属于同一 GCShape 组
// 编译器为每个 GCShape 生成代码实例，并在每次调用时以字典传递类型信息
type a int
type b int
type c = int

func restrictPrint[T ~uint32 | ~float64, E ~[]T] (x T, e E) {
	for _, v := range e {
		fmt.Println(v)
	}
}

func test[T any](x T) {
	println(x)
}


func PrintString[T fmt.Stringer](x T) {
	fmt.Println(x.String()) // 这里调用接口的方法
}

type Persion struct {
	Name string
}

func (p Persion) String() string {
	return "Persion: " + p.Name
}

type Dog struct {
	Name string
}

func (d Dog) String() string {
	return "Dog: " + d.Name
}

func main() {
	restrictPrint(100, []uint32{1, 2, 3})
	restrictPrint(3.14, []float64{1.1, 2.2, 3.3})
	println("===============>")
	
	// CALL main.test[go.shape.int](SB)
	test(1)

	type X int
	// CALL main.test[go.shape.int](SB)
	// 这里int和X有相同的GCShape->编译器只生成一个实例化版本
	test(X(2))
	// 这里string和X有不同的GCShape->编译器生成不同的实例化版本
	// CALL main.test[go.shape.string]
	test("abc")

	println("===============>")

	// GCShape分析
	// Person和Dog都是struct结构体,它们字段布局指针mask相同,属于同一组GCShape
	// 或者直接改成 *Persion和*Dog
	// 这时候只会产生一份stenciled函数体
	// 共享的函数在编译时根本就不知道 x.String() 应该调用 Persion.String 还是 Dog.String
	// 编译器会为每个具体实例化生成一个静态的dictionary结构体(在二进制的数据段中)
	// 查表查看具体要调用哪个结构体中的方法
	PrintString(Persion{Name: "John"})
	PrintString(Dog{Name: "Rex"})
}

// 关于 **所有类型的指针都属于同一个GCShape,共享同一份代码实例**

// Go 泛型的实现是 GCShape + Dictionary 的混合策略
// 对于值类型而言(int struct...):性能非常好,几乎零开销,能完全内联
// 对于指针类型、接口类型: 所有 *T（不管T是什么）都属于同一个GCShape（通常情况下是 *uint8）,只生成一份 stenciled代码
// 所有指针类型、接口类型共享，导致需要一个Dictionary来传递itab和rtype，运行时动态分发
// 导致的结果就是 性能退化到和 interface{} 差不多了,甚至在某些情况下反射重的情况更慢一些
