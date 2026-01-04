package main

func test(a... int) {
	for i := range a {
		a[i] += 100
	}
}

//go:noinline
func closure() func() {
	x := 100
	println(&x, x)
	return func() {
		x++
		println(&x, x)
	}
}

func testDefer() (z int) {
	defer func() {
		z += 200
	}()
	return 100
}

func testDefer2() int {
	z := 100
	
	defer func() {
		z += 200
	}()

	z = 100
	return z
}

func main() {
	println("===============>")
	a := []int{1, 2, 3}
	test(a...)
	println(a[1])
	println("===============>")
	closure()()
	println("===============>")
	println(testDefer())
	println("===============>")
	println(testDefer2())
}
