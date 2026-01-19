## Net

### TCP

传输控制协议 TCP 是一种面向连接、可靠、基于字节流（stream）的传输层通信协议

- 包编号，接收端确认（ACK），需要时重传
- 数据校验，确保传输过程中不会出错

### UDP

用户数据报协议 UDP 是无连接、不可靠、不关心后续分组状态

- 协议不可靠，不保证数据报不丢失、不延迟、不错序传输
- 客户端不关心服务端是否处于监听或者已经准备好接受数据
- 无连接协议可方便实现一对多，或多对一通信

实际上作为TCP、UDP基础的IP协议也是无连接的，TCP分组（segment）是通过自身来实现可靠性的

多数情况下UDP要比TCP更快

考虑到UDP协议的不可靠特性，所以包大小最好限制在 MTU 范围内，需要减去相关协议头大小

```text
Internet（广域网）: MTU(576) - IPv4(20) - UDP(8) = 548
Intranet（局域网）: MTU(1500) - PPPoE(8不是所有Intranet都存在) - IPv4(20) - UDP(8) = 1464
```

鉴于 IP 头最大值是 60，因此建议的Internet安全上限为 508 字节

同理 Intranet 合理阈值为 1424 字节

### HTTP
超文本传输协议（HyperText Transfer Protocol）

由客户端发起请求，创建一个到服务器指定端口（默认80）的TCP连接，服务器返回状态以及内容。所请求资源，由统一资源标识符（URI）标识

HTTP/2，简称 h2（加密）或 h2c（非加密），基于SPDY协议改进

- 二进制编码
- 多路复用，在同一个连接内合并多个请求
- 流水线，批量提交请求
- 压缩报文头（HPACK）
- 服务端推送（PUSH）

HTTP/3 弃用 TCP 协议，改为基于 UDP 的 QUIC 协议实现

**请求方法:**

- GET: 显示请求
- HEAD: 只传回头消息
- POST: 提交数据（创建）
- PUT: 更新内容（更新或创建）
- DELETE: 删除资源

```go
package main

import (
	"fmt"
	"log"
	"net/http"
	"time"
)

type Hello struct {}

func (*Hello) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	fmt.Fprintf(w, "hello, world!")
}

func main() {
	srv := &http.Server{
		Addr: ":8080",
		Handler: &Hello{},
		ReadTimeout: 10 * time.Second,
		WriteTimeout: 10 * time.Second,
		IdleTimeout: 10 * time.Second,
	}

	log.Fatal(srv.ListenAndServe())
}
```

**handler**

处理器方法的两个参数，分别对应返回和请求数据

```go
type Handler interface {
	ServeHTTP(ResponseWriter, *Request)
}
```

- Request: 客户端发送的请求信息
- ResponseWriter: 返回给客户端的数据

- 处理器不应修改请求数据
- 处理器结束后，会进行资源清理，不应再持有或访问这两个参数

```go
type TestHandler struct {}

func (*TestHandler) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	// request
	fmt.Println("method:", req.Method)
	fmt.Println("   url:", req.URL)
	fmt.Println("       ", req.URL.Query())
	
	fmt.Println("header:", req.Header)
	for k, v := range req.Header {
		fmt.Println(k, ":", v)
	}

	fmt.Println("  body:", req.Body)
	fmt.Println("       ", string(data))

	// response
	w.Header().Set("test", "demo server")
	w.WriteHeader(http.StatusCreated)
	w.Write([]byte("hello, world!"))
}

func main() {
	log.Fatalln(http.ListenAndServe(":8080", &TestHandler{}))
}
```

**参数**

对连接和请求数据进行处理，包装成 `Request` 和 `response` 对象

请求结束时关闭和清理资源，不该继续持有或访问

```go
// http/server.go

func (c *conn) readRequest(ctx context.Context) (w *response, err error) {
	req, err := readRequest(c.bufr)
	w = &response{
		req: req,
		reqBody: req.Body,
	}
	return w, nil
}

func (w *response) finishRequest() {
	w.cw.close()
	w.reqBody.Close()
}
```

在 response.Write 内检查并调用 WriteHeader， 而后者不允许重复设定

所以，要范围非 200 状态码，必须在 Write 之前调用

```go
// http/server.go

func (w *response) write(int, []byte, string) (n int, err error) {
	if !w.wroteHeader {
		w.WriteHeader(StatusOK)
	}
}

func (w *response) WriteHeader(code int) {
	if w.wroteHeader {
		w.conn.server.logf("http: superfluous ...", ...)
		return
	}

	w.wroteHeader = true
	w.status = code
}
```

**路由**

自带的 `ServeMux` 路由只能算一个范例，功能很弱，正式项目中很少使用。其作用是统一接收请求，然后按路径模式分发给其他已注册处理器

```go
// http/server.go

type ServeMux struct {
	m	map[string]muxEntry
	es	[]muxEntry
}

func (mux *ServeMux) Handle(pattern string, handler Handler) {
	mux.mu.Lock()
	defer mux.mu.Unlock()

	if _, exist := mux.m[pattern]; exist {
		panic("http: multiple registrations for " + pattern)
	}

	if mux.m == nil {
		mux.m = make(map[string]muxEntry)
	}

	e := muxEntry{h: handler, pattern: pattern}
	mux.m[pattern] = e

	if pattern[len(pattern)-1] == '/' {
		mux.es = append(mux.es, e)
	}
}

func (mux *ServeMux) ServeHTTP(w http.ResponseWriter, r *http.Request) {

	h, _ := mux.Handler(r)

	h.ServeHTTP(w, r)
}


func (mux *ServeMux) Handler(r *http.Request) (h Handler, pattern string) {
	return mux.handler(host, r.URL.Path)
}

func (mux *ServeMux) handler(host, path string) (h Handler, pattern string) {
	if h == nil {
		h, pattern = mux.match(path)
	}
	if h == nil {
		h, pattern = NotFoundHandler, ""
	}
	return
}

func (mux *ServeMux) match(path string) (h Handler, pattern string) {
	v, ok := mux.m[path]
	if ok {
		return v.h, v.pattern
	}

	for _, e := range mux.es {
		if strings.HasPrefix(path, e.pattern) {
			return e.h, e.pattern
		}
	}

	return nil, ""
}
```

**处理器**

- NotFound, NotFoundHandler: 404
- Redirect, RedirectHandler: 301, 302, 303
- TimeoutHandler: 超时处理
- StripPrefix: 移除前缀
- FileServer: 文件服务
- httputil/ReverseProxy: 反向代理

**client**

- 并发安全。如果要复用连接，应该使用单一实例
- 直接用 `DefaultClient` 提供的便捷函数
- 必须调用 `resp.Body.Close`

```go
func main() {
	resp, _ := http.Get("http://www.baidu.com")
	defer resp.Body.Close()

	body, _ := io.ReadAll(resp.Body)
	fmt.Println(string(body))
}
```

**h2,tls**

传输层安全性协议（TLS），其前身 安全套接层（SSL）是一种安全协议，目的是为了通信提供安全以及数据完整性保障

**基本过程**

1. 客户端获取证书公钥

2. 协商生成「对话密钥」

3. 双方使用「对话密钥」加密数据

```bash
openssl genrsa -out key.pem 2048
openssl req -new -x509 -key key.pem -out cert.pem -days 365
```

```go
package main

import (
	"fmt"
	"log"
	"net/http"
)

func hello(w http.ResponseWriter, req *http.Request) {
	fmt.Fprintf(w, "hello, world!")
}

func main() {
	http.HandleFunc("/hello", hello)
	log.Fatal(http.ListenAndServeTLS(":8080", "cert.pem", "key.pem", nil))
}
```

以 TLS 启动后，默认支持 HTTP/2（h2）协议

```bash
curl -v --insecure https://localhost:8080/hello
```

```go
package main

import (
	"crypto/tls"
	"fmt"
	"log"
	"net/http"
	"net/http/httputil"
)

func main() {
	trans := http.DefaultTransport.(*http.Transport).Clone()
	trans.TLSClientConfig = &tls.Config{
		InsecureSkipVerify: true,
	}

	client := &http.Client{
		Transport: trans,
	}

	resp, err := client.Get("https://localhost:8080/hello")
	if err != nil {
		log.Fatalln(err)
	}
	defer resp.Body.Close()

	body, _ := io.ReadAll(resp.Body)
	fmt.Println(string(body))
}
```

### URL

统一资源定义符（URL）格式:

`scheme:[//[user[:password]@]host[:port]][/path][?query][#fragment]`

```text
http://example.com:80/path/to/my.html?k1=v1&k2=v2#some
| 1 | |     2     |3|       4       |     5    |  6  |

1: 协议 protocol (http/https/ftp/file)
2: 域名 domain
3: 端口 port
4: 路径 path
5: 参数 query
6: 锚点 fragment（片段标识符）
```

```go
func main() {
	s := "http://example.com:80/path/to/my.html?k1=v1&k2=v2#some"

	u, _ := url.Parse(s)
	u2, _ := url.ParseRequestURI(s)

	fmt.Printf("%#v\n%#v\n", u, u2)
}
```

### RPC

**远程调用**，是种通信协议，允许像调用本地代码那样去调用另一台主机的服务。标注库实现了基于TCP、HTTP协议的远程调用

**服务限制**

- 必须是导出类型
- 必须是导出方法
- 参数:
	- 第一参数为调用参数（通常使用复合类型指针）
	- 第二参数是返回值指针
	- 必须是导出类型
	- 可被序列化，默认`encoding/gob`
- 返回值必须是 error

#### TCP
标准库提供了默认服务器（`DefaultServer`）和编写函数

接入方法`Accept`为每个客户端建立一个 goroutine 服务

```go
package main

import (
	"fmt"
	"log"
	"net"
	"net/rpc"
)

type Service struct {}

func (s *Service) Add(args []int, reply *int) error {
	*reply = args[0] + args[1]
	return nil
}

func server() {
	l, err := net.Listen("tcp", ":8181")
	if err != nil {
		log.Fatalln(err)
	}

	srv := rpc.NewServer()
	srv.RegisterName("my", &Service{})

	go srv.Accept(l)
}

func test() {
	client, err := rpc.Dial("tcp", "localhost:8181")
	if err != nil {
		log.Fatalln(err)
	}

	var reply int
	err = client.Call("my.Add", []int{1, 2}, &reply)
	if err != nil {
		log.Fatalln(err)
	}
	fmt.Println(reply)
}

func main() {
	log.SetFlags(log.Lshortfile)

	server()

	test()
}
```

客户端可使用异步模式

```go
func test() {
	client, err := rpc.Dial("tcp", "localhost:8181")
	if err != nil {
		log.Fatalln(err)
	}
	defer client.Close()

	var x int
	call := client.Go("my.Add", []int{1, 2}, &x, nil)

	if reply := <- call.Done; reply.Error != nil {
		log.Fatalln(err)
	}

	fmt.Println(x)
}
```

#### HTTP

除了协议不同外，其他逻辑基本一致

```go
package main

import (
	"fmt"
	"log"
	"net/http"
	"net/rpc"
	"time"
)

type Service struct {}

func (s *Service) Add(args []int, reply *int) error {
	*reply = args[0] + args[1]
	return nil
}

func server() {
	srv := rpc.NewServer()
	srv.RegisterName("my", &Service{})

	http.Handle("/rpc", srv)
	go http.ListenAndServe(":8181", nil)
}

func test() {
	client, err := rpc.DialHTTPPath("tcp", "127.0.0.1:8181", "/rpc")
	if err != nil {
		log.Fatalln(err)
	}
	defer client.Close()

	var x int
	if err := client.Call("my.Add", []int{1, 2}, &x); err != nil {
		log.Fatalln(err)
	}
	fmt.Println(x)
}

func main() {
	log.SetFlags(log.Lshortfile)

	server()
	time.Sleep(time.Second * 10)
	test()
}
```