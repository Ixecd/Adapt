package main

import (
	"context"
	"net"
	"sync"
	"time"
)

const (
	network  = "tcp"
	address  = ":8088"
	packsize = 8

	// 客户端连接建立（三次握手），快速失败，防止慢节点拖累
	dialTimeout   = time.Second
	// 服务端等待新连接，防止事件循环无限阻塞
	acceptTimeout = time.Second * 10
	// 连接双活，检测空闲/异常连接，释放资源
	connTimeout   = time.Second * 20
)

func server(ctx context.Context) <-chan struct{} {
	ready, shutdown := make(chan struct{}), make(chan struct{})

	go func() {
		defer func() { recover() }()
		defer close(shutdown)

		srv := checkFatal(listenx(network, address)) // reusepool 实现
		defer srv.Close()

		close(ready)

		var wg sync.WaitGroup
		defer wg.Wait()

		for {
			select {
			case <-ctx.Done():
				return
			default:
			}

			// accept
			setDeadline(srv, acceptTimeout)
			conn, err := srv.AcceptTCP()

			if err != nil {
				if isTimeout(err) {
					continue
				}
				logFatal(err)
			}

			// client
			wg.Add(1)
			go func() {
				defer func() { recover() }()
				defer wg.Done()
				defer conn.Close()

				defer logPrint("closed:", conn.RemoteAddr())
				logPrint("connect:", conn.RemoteAddr())

				handleConn(ctx, conn)
			}()
		}
	}()

	select {
	// 监听成功
	case <-ready:
	// 不无限等待，防止调用者卡死，预期的是 server(ctx) 调用后应该 立即返回 不阻塞主线程
	// 让主线程继续做其他事情（注册信号、启动其他服务等）
	case <-time.After(time.Second * 2):
	}

	return shutdown
}

func handleConn(ctx context.Context, conn *net.TCPConn) {
	pack := make([]byte, packsize)
	for {
		select {
		case <-ctx.Done():
			return
		default:
		}

		if data := connRead(conn, pack); len(data) > 0 {
			connWrite(conn, data)
		}
	}
}
