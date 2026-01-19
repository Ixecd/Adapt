package main

import (
	"context"
	"time"
)

func main() {
	ctx, cancel := context.WithCancel(context.Background())

	done := server(ctx)

	go func() {
		time.Sleep(time.Second * 60)
		cancel()
	}()

	for i := 0; i < 10; i++ {
		go client()
	}

	<-done
}