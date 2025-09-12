package main

import (
	_ "time"
)

type Result struct {
	value string
}

type Search func(q string) Result


func First(query string, replicas ...Search) Result {
	c := make(chan Result)
	searchReplicas := func(i int) { c <- replicas[i](query) }
	for i := range replicas {
		go searchReplicas(i)
	}
	return <-c
}