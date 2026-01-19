package main

import (
	"fmt"
	"math/rand"
	"net"
	"time"
)

func client() {
	defer func() { recover() }()

	conn := checkFatal(net.DialTimeout(network, address, dialTimeout))
	defer conn.Close()

	testdata(conn)
}

func testdata(conn net.Conn) {
	pack := make([]byte, packsize)

	for {
		rand.Read(pack)
		_ = connWrite(conn, pack)

		b := connRead(conn, pack)
		if len(b) > 0 {
			fmt.Printf("recv: %d bytes\n", len(b))
		}

		time.Sleep(time.Millisecond * 1000)
	}
}