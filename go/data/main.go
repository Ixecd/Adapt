package main

import "fmt"

func main() {
	x := [3][2]int{
		{1, 2},
		{3, 4},
		{5, 6},
	}

	fmt.Printf("len(x) = %d", len(x))
	fmt.Printf("cap(x) = %d", cap(x))
	fmt.Printf("len(x[0]) = %d", len(x[0]))
	fmt.Printf("cap(x[0]) = %d\n", cap(x[0]))

	println("================================================")
	d := [...]int{0, 1, 2, 3}

	var p *[4]int = &d
	var pe *int = &d[1]

	p[0] += 10
	*pe += 20

	fmt.Println(d)

	println("================================================")

	arr := [...]int{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
	s := arr[2:6:8]
	fmt.Println(s)

	println("================================================")

	m := make([]int, 3, 4)
	a := append(m, 1)
	b := append(m, 2)
	fmt.Println(m, a, b)

	println("================================================")

	var mp map[string]int

	_ = mp["a"]
	delete(mp, "a")
	// mp["b"] = 2

	println("================================================")
	
	ap := map[string]int{
		"a": 1,
	}
	ap["a"]++

	fmt.Println(ap)
}