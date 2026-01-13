## sort
让目标对象实现特定接口，以支持排序

内部实现了 QuickSort、HeapSort、InsertionSort、SymMerge算法

## Slice
通过自定义函数，选择要比较的内容，或改变次序

```go
func Slice(x any, less func(i, j int) bool)

func SliceStable(x any, less func(i, j int) bool)

func SliceIsSorted(x any, less func(i, j int) bool)
```

## Interface
避开辅助函数，实现排序接口

```go
type Interface interface {
	Len() int
	Less(i, j int) bool
	Swap(i, j int)
}
// 不稳定排序，不保证相等元素原始次序不变
func Sort(data Interface)
// 稳定排序，保证相等元素原始次序不变
func Stable(data Interface)
```

## Search
排序后的数据，可用Search进行二分搜索。返回 `[0,n)` 范围内，`f() == true` 的最小索引序号

```go
func Search(n int, f func(int) bool) int
```

## Reverse
辅助函数 Reverse 返回一个将 Less 参数对调的包装对象
```go
// sort.go
type reverse struct {
	Interface
}

func (r reverse) Less(i, j int) bool {
	return r.Interface.Less(j, i)
}

func Reverse(data Interface) Interface {
	return &reverse{data}
}
```