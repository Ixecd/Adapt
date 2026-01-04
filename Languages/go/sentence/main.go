package main

import (
	"fmt"
	"log"
	"errors"
)

func a() {}

func b() {}

func do(x int) error {
	if x <= 0 {
		return errors.New("x <= 0")
	}
	return nil
}

func testSwitch() error {
	a, b, c := 1, 2, 3
	switch x := 5; x {
	case a, b:
		println("a | b")
	case c:
		println("c")
	case 4:
		println("d")
	default:
		println(x)
	}
	return nil
}

func testFallthrough() error {
	switch x := 5; x {
	default:
		println("ok")
	case 5:
		println("5")
		fallthrough
	case 6:
		println("6")
	case 7:
		println("7")
	}

	switch x := 1; {
	case x == -1, x == 1:
		println("a")
	case x > 1 && x <= 10:
		println("a")
	case x > 10:
		println("b")
	default:
		println("z")
	}
	return nil
}

type data struct {
	x int
	s string
}

func exec(f func()) {
	f()
}

func testData () {
	// var a data = data{1, "abc"}
	// a2 := data{1, "abc"}

	// b := data{
	// 	1,
	// 	"abc",
	// }

	// c := []int{
	// 	1,
	// 	2,
	// }

	// d := []int{
	// 	1,2,3,4,
	// 5}
}

func main() {
	x := 1
	err := do(x)

	if err != nil {
		log.Fatalln(err)
	}

	x++
	println(x)
	fmt.Println("=====================>")
	testSwitch()

	fmt.Println("=====================>")
	testFallthrough()

	println(a == nil)

	fmt.Println("=====================>")
	var f func() = func() { println("hello,world!") }
	exec(f)
}
