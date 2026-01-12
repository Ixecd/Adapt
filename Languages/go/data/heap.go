package main

import (
	"container/heap"
	"fmt"
)

type Message struct {
	test string
	priority int
}

type Queue []Message

func (q Queue) Len() int {
	return len(q)
}

// 最大堆的话，下面是 >
func (q Queue) Less(i, j int) bool {
	return q[i].priority < q[j].priority
}

func (q Queue) Swap(i, j int) {
	q[i], q[j] = q[j], q[i]
}

func (q *Queue) Push(x any) {
	*q = append(*q, x.(Message))
}

func (q *Queue) Pop() any {
	n := len(*q)
	x := (*q)[n - 1]
	*q = (*q)[:n - 1]
	return x
}

func main() {
	var q Queue = []Message{
		{"a3", 3},
		{"a1", 1},
		{"a2", 2},
		{"a4", 4},
		{"a5", 5},
		{"a6", 6},
		{"a7", 7},
		{"a8", 8},
		{"a9", 9},
		{"a10", 10},
	}

	heap.Init(&q)
	heap.Push(&q, Message{"a0", 0})

	q[0].priority += 10
	heap.Fix(&q, 0)

	fmt.Println(q)

	for q.Len() > 0 {
		fmt.Println(heap.Pop(&q).(Message))
	}
}