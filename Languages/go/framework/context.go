package main

import (
	"context"
	"fmt"
	"time"
)

func request(ctx context.Context) {
	ctx, cancel := context.WithTimeout(ctx, time.Second)
	defer cancel()

	resp := make(chan int)
	go handle(ctx, resp)

	// do something

	select {
	case v := <-resp:
		println(v)
	case <-ctx.Done():
		println(ctx.Err().Error())
	}
}

func handle(ctx context.Context, resp chan<- int) {
	println("1/3 handle")
	cache(ctx, resp)
}

func cache(ctx context.Context, resp chan<- int) {
	println("2/3 cache")

	time.Sleep(time.Second * 2)

	database(ctx, resp)
}

func database(ctx context.Context, resp chan<- int) {
	select {
	case <-ctx.Done():
		println("3/3 database: timeout!")
		return
	default:
	}

	println("3/3 database")
}

func test() {

	// chain: request -> handle -> cache -> database
	request(context.Background())

	fmt.Scanln()
}
