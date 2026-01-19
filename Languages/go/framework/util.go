package main

import (
	"context"
	"errors"
	"io"
	"log"
	"net"
	"syscall"
	"time"

	// 访问低级syscall，比如Setsockopt
	"golang.org/x/sys/unix"
)

func init() {
	log.SetFlags(log.Ltime)
}

func logPrint(v ...any) {
	log.Println(v...)
}

func logFatal(v ...any) {
	logPrint(v...)
	panic("fatal")
}

func checkFatal[T any](v T, err error) T {
	if err != nil {
		logFatal(err)
	}

	return v
}

func setDeadline(c any, d time.Duration) {
	t := time.Now().Add(d)

	switch v := c.(type) {
	case *net.TCPListener:
		v.SetDeadline(t)
	case net.Conn:
		v.SetDeadline(t)
	}
}

func isTimeout(err error) bool {
	e, ok := err.(net.Error)
	return ok && e.Timeout()
}

func connError(conn net.Conn, err error) {
	if err == nil {
		return
	}

	switch {
	case isTimeout(err),
		err == io.EOF,
		errors.Is(err, syscall.ECONNRESET),
		errors.Is(err, syscall.EPIPE):
		panic(err)
	}

	logPrint(err)
}

func connRead(conn net.Conn, buf []byte) []byte {
	setDeadline(conn, connTimeout)
	n, err := conn.Read(buf)

	if err != nil {
		connError(conn, err)
	}

	return buf[:n]
}

func connWrite(conn net.Conn, b []byte) int {
	setDeadline(conn, connTimeout)
	n, err := conn.Write(b)

	if err != nil {
		connError(conn, err)
	}

	return n
}

func listenx(network, address string) (*net.TCPListener, error) {
	lc := net.ListenConfig{
		Control: func(network, address string, c syscall.RawConn) error {
			return c.Control(func(fd uintptr) {
				unix.SetsockoptInt(int(fd), unix.SOL_SOCKET, unix.SO_REUSEADDR, 1)
				_ = unix.SetsockoptInt(int(fd), unix.SOL_SOCKET, unix.SO_REUSEPORT, 1)
			})
		},
	}

	ln, err := lc.Listen(context.Background(), network, address)
	if err != nil {
		return nil, err
	}

	return ln.(*net.TCPListener), nil
}
