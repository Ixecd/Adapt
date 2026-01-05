package main

import "fmt"
// 声明汇编函数原型
func add(x, y int64) (z int64)
func main() {
	z := add(0x100, 0x200)
	fmt.Println(z)
}