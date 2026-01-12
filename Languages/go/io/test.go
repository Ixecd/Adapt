package main

import (
	_ "bufio"
	"io"
	"os"
)

func main() {
	f, _ := os.Open("./tmp.dat")
	defer f.Close()

	// 直接从os.File.Read中读取的话，会导致系统调用非常频繁
	var r io.Reader = f
	// bufio 先从系统调用中预读 8192 字节放到自己的缓存里，然后每次从缓存取512字节，系统调用的次数会大大减少
	// r = bufio.NewReaderSize(r, 8192)

	for {
		buf := make([]byte, 512)
		_, err := r.Read(buf)
		if err == io.EOF {
			break
		}
	}
}