## SQL

### DB

数据库抽象，用于维持会话，执行创建、查询、事务等操作

- 内部连接池管理多个底层链接。按需创建、复用和释放
- 并发安全实现，可建立长生命周期实例（全局变量）

使用前，需导入驱动
```go
package main

import (
	"database/sql"
	"fmt"
	"log"

	_ "github.com/mattn/go-sqlite3" // 导入驱动
)

func main() {
	log.SetFlags(log.Lshortfile)

	db, err := sql.Open("sqlite3", ":memory:")
	if err != nil {
		log.Fatalln(err)
	}
	defer db.Close()

	if err := db.Ping(); err != nil {
		log.Fatalln(err)
	}

	fmt.Println("%+v\n", db.Stats())
}
```

「源码剖析」

先是注册驱动

```go
// database/sql/sql.go

var (
	driversMu sync.Mutex
	drivers = make(map[string]driver.Driver)
)

func Register(name string, driver driver.Driver) {
	driversMu.Lock()
	defer driversMu.Unlock()

	if driver == nil {
		panic("sql: Register driver is nil")
	}

	if _, dup := drivers[name]; dup {
		panic("sql: Register called twice for driver " + name)
	}

	drivers[name] = driver
}
```

导入驱动，其初始化函数会调用 Register 函数进行注册

```go
// sqlite3.go

var driverName = "sqlite3"

func init() {
	if driverName != "" {
		sql.Register(driverName, &SQLiteDriver{})
	}
}
```

可用 Drivers 函数返回已注册驱动列表

```go
// database/sql/sql.go

func Drivers() []string {
	driversMu.RLock()
	defer driversMu.RUnlock()

	list := make([]string, 0, len(drivers))
	for name := range drivers {
		list = append(list, name)
	}

	sort.Strings(list)
	return list
}
```

另外 Open 不会立即创建连接

```go
func Open(driverName string, dataSourceName string) (*DB, error) {
	driversMu.RLock()
	driveri, ok := drivers[driverName]
	driversMu.RUnlock()

	if !ok {
		return nil, fmt.Errorf("unknown driver %q", driverName)
	}

	if driverCtx, ok := driveri.(driver.DriverContext); ok {
		connector, err := driverCtx.OpenConnector(dataSourceName)
		if err != nil {
			return nil, err
		}
		return OpenDB(connector), nil
	}
	return OpenDB(dsnConnector(dsn: dataSourceName, driver: driveri)), nil
}

func OpenDB(c driver.Connector) *DB {
	ctx, cancel := context.WithCancel(context.Background())
	db := &DB{
		connector: c,
		openerCh: make(chan struct{}, connectionRequestQueueSize),
		lastPut: make(map[*driverConn]string),
		connRequests: make(map[uint64]chan connRequest),
		stop: cancel,
	}

	go db.connectionOpener(ctx)

	return db
}
```

### connection

通常不需要显式获取连接，由相关方法自动完成

- 优先从连接池获取空闲连接
- 如果没取到，检查是否达到上限，以决定是否新建连接
- 已达上限，请求被阻塞，直到有可用连接被放回池中

- 方法 Conn 获取连接，需用 conn.Close 放回
- 方法 Stats 返回相关状态 统计信息

闲置过久会出问题，可设置生命周期。连接池也会自动清理坏掉的连接。如果相关方法检测到 ErrBadConn，会重新获取连接重试

- SetMaxOpenConns: inuse + idle
- SetMaxIdleConns: MaxIdle <= MaxOpen
- SetConnMaxIdleTime
- SetConnMaxLifeTime
- SetMaxIdleConns(0): 释放后，重新设置

```go
func (dc *driverConn) releaseConn(err error) {
	// 这里叫put，其实是为了连接池复用链接，并不会真正release，而是put到空闲队列中
	dc.db.putConn(dc, err, true)
}
```

**信号**

在 「OpenDB」 里有个特殊的操作，接收信号，创建新连接并放回池中

```go
// database/sql/sql.go

func OpenDB(c driver.Connector) *DB {
	go db.connectionOpener(ctx)
}

func (db *DB) connectionOpener(ctx context.Context) {
	for {
		select {
			case <- db.openerCh:
				db.openNewConnection(ctx)
		}
	}
}

func (db *DB) openNewConnection(ctx context.Context) {
	// 新建连接
	ci, err := db.connector.Connect(ctx)
	dc := &driverConn{
		db: db,
		ci: ci,
	}

	// 放回连接池
	if db.putConnDBLocked(dc, err) {
		db.addDepLocked(dc, dc)
	} else {
		ci.Close()
	}
}

// 谁发的信号？什么时候发的？？

// database/sql/sql.go
func (db *DB) putConn(dc *driverConn, err error, resetSession bool) {
	// 如果放回的链接是坏的
	if errors.Is(err, driver.ErrBadConn) {
		db.maybeOpenNewConnections()
		dc.Close()
		return
	}
	// ...
}

func (db *DB) maybeOpenNewConnections() {
	// 按被阻塞的请求数，发送信号
	numRequests := len(db.connRequests)
	if db.maxOpen > 0 {
		numCanOpen := db.maxOpen - db.numOpen
		if numRequests > numCanOpen {
			numRequests = numCanOpen
		}
	}

	for numRequests > 0 {
		db.numOpen++
		numRequests--
		if db.closed { return }
		
		db.openerCh <- struct{}{}
	}
}

func (dc *driverConn) finalClose() error {
	dc.db.maybeOpenNewConnections()
}
```

### query

执行 SELECT 查询，从数据库返回记录

- 尽早调用 `Rows.Close` 归还连接
- 调用 `Rows.Err` 检查 `Next` 是否意外终止
- 多结果集需数据库支持（`Rows.NextResultSet`）

```go
package main

import (
	"database/sql"
	"log"

	_ "github.com/mattn/go-sqlite3"
)

func main() {
	log.SetFlags(log.Lshortfile)

	db, err := sql.Open("sqlite3", "./test.db")
	if err != nil {
		log.Fatalln(err)
	}
	defer db.Close()

	q := `SELECT name FROM user WHERE id > ?`

	rows, err := db.Query(q, 0)
	if err != nil {
		log.Fatalln(err)
	}

	// 出错会导致迭代中断
	if rows.Next() {
		var name string

		if err := rows.Scan(&name); err != nil {
			log.Fatalln(err)
		}

		println(name)
	}

	// 尽早关闭，归还连接
	if err := rows.Close(); err != nil {
		log.Fatalln(err)
	}

	// 检查 Next 错误（可在Close之后）
	// 其职责是返回迭代过程中累计的任何错误
	// Err和Close操作是正交的 关闭资源 != 清除错误状态
	if err := rows.Err(); err != nil {
		log.Fatalln(err)
	}
}
```

### queryRow

对 Query 进行包装，用于返回单条记录

- 没找到记录，返回ErrNoRows
- 方法 Row.Scan 主动释放连接

```go
package main

import (
	"database/sql"
	"log"

	_ "github.com/mattn/go-sqlite3"
)

func main() {
	log.SetFlags(log.Lshortfile)

	db, err := sql.Open("sqlite3", "./test.db")
	if err != nil {
		log.Fatalln(err)
	}
	defer db.Close()

	q := `SELECT name FROM user WHERE id > ?`

	var name string
	err = db.QueryRow(q, 0).Scan(&name)

	if err == sql.ErrNoRows {
		println("norow")
		return
	} else if err != nil {
		log.Fatalln(err)
	}

	println(name)
}
```

### exec

执行 INSERT、UPDATE、DELETE 等无记录返回的操作

- 方法 `Exec` 自动释放连接
- 对结果 `Result` 处理，无需持有连接

```go
package main

import (
	"database/sql"
	"log"

	_ "github.com/mattn/go-sqlite3"
)

func main() {
	log.SetFlags(log.Lshortfile)

	db, err := sql.Open("sqlite3", "./test.db")
	if err != nil {
		log.Fatalln(err)
	}
	defer db.Close()

	q := `INSERT INTO user (id, name) VALUES (?, ?)`

	result, err := db.Exec(q, 3, "u3")
	if err != nil {
		log.Fatalln(err)
	}

	id, err := result.LastInsertId()
	if err != nil {
		log.Fatalln(err)
	}
	println(id)

	rows, err := result.RowsAffected()
	if err != nil {
		log.Fatalln(err)
	}

	println(rows)
}
```

「源码剖析」

在 exec 退出前，释放连接

```go
// database/sql/sql.go

func (db *DB) exec(ctx context.Context, query string, args []interface{}, startegy *TxOptions) (Result, error) {
	dc, err := db.conn(ctx, strategy)

	return db.execDC(ctx, dc, dc.releaseConn, query, args)
}

func (db *DB) execDC(ctx context.Context, dc *driverConn, releaseConn func(error), query string, args []interface{}) (Result, error) {
	// 确保归还连接
	defer func() {
		release(err)
	}()

	// 检查接口实现
	execerCtx, ok := dc.ci.(driver.ExecerContext)

	if !ok {
		withLock(dc, func() {
			resi, err = ctxDriverExec(ctx, execerCtx, execer, query, nvdargs)
		})

		if err != driver.ErrSkip {
			if err != nil {
				return nil, err
			}
			return driverResult{dc, resi}, nil
		}
	}

	// 以 Perpare 方式执行
	withLock(dc, func() {
		si, err = ctxDriverPrepare(ctx, dc.ci, query)
	})

	ds := &driverStmt{dc: dc, si: si}
	defer ds.Close()

	return resultFromStatement(ctx, dc.ci, ds, args...)
}
```

### perpare

所谓 `prepare statement` 是指数据库对SQL进行预处理（检查、优化、编译等）

**优点**

- 占位符加参数方式更方便，可以防止「SQL注入」等问题
- 某些数据库使用二进制协议。相比文本，效率更高
- 用于多次执行，消除了解析（检查SQL语法是否正确、分析语句结构，构建抽象语法数、验证表名、列名是否存在，权限是否足够）等开销，性能更好

**缺点**

- 首次预处理时间较长
- 多次网络通信开销（prepare/ exec/ close）

**占位符**

- MySQL: ?
- PostgreSQL: $1, $2
- SQLite: ? OR $1, $2

- 某些驱动，将含参数操作一律当 Prepare 执行
- 不含参数的 Simple Exec 可能有更好的性能
- 应及时调用 Stmt.Close 释放资源

```go
package main

import (
	"database/sql"
	"fmt"
	"log"

	_ "github.com/mattn/go-sqlite3"
)

func main() {
	log.SetFlags(log.Lshortfile)

	// 返回的其实是一个连接池，内部管理多个底层数据库连接
	db, err := sql.Open("sqlite3", "./test.db")
	if err != nil {
		log.Fatalln(err)
	}
	// 关闭整个连接池
	defer db.Close()

	q := `INSERT INTO user (id, name) VALUES (?, ?)`

	stmt, err := db.Prepare(q)
	if err != nil { log.Fatalln(err) }

	for i := 0; i < 10; i++ {
		result, err := stmt.Exec(i, fmt.Sprintf("u%d", i))
		if err != nil { log.Fatalln(err) }

		fmt.Println(result.RowsAffected())
	}

	// 释放这个 prepared statement 资源
	if err := stmt.Close(); err != nil { log.Fatalln(err) }
}
```

#### prepared statement 到底占不占用连接？

不独占链接

- 当调用 db.Prepare() 时，底层会临时借一个连接从池子里，去执行 prepare 命令
- 准备完成后，这个连接会「立即放回池子」
- 之后每次 `stmt.Exec()` 或 `stmt.Query()`时，又会「临时借一个连接」来绑定参数的命令
- 所以stmt本身不「捏着」一个连接不放，它只是一个**语句模块**，执行时动态借链接

#### 为什么用了stmt就要额外Close，而直接Exec就不用？

使用 Prepare + Stmt:

- 显式创建一个长期存在的 `*sql.Stmt` 对象
- 这个对象持有「客户端侧的缓存/句柄」、「服务器端的prepared statement资源」
- 如果不stmt.Close()，这些资源不会立即释放（尤其是服务器端），长期下来可能累积泄露

不使用 Stmt，直接db.Exec() / db.Query():

- 每次调用都是**隐式的一次 prepare + exec**，底层自动借用连接 -> prepare -> bind参数 -> exec -> 自动清理语句 -> 放回连接


### transaction

事务，在单个逻辑单元内的一系列操作，All or Nothing

基本特性（ACID）

- 原子性（Atomicity）：作为整体被执行
- 一致性（Consistency）：确保从一致状态转变为另一个一致状态
- 隔离性（Isolation）：多事务执行时，彼此隔离，互不干扰
- 持久性（Durability）：已提交修改应持久化保存

因为连接池的缘故，多个 DB.Exec 调用无法保证在同一连接上完成。所以不能直接 DB.Exec("BEGIN") 来启动或提价事务。应该通过 Begin 创建事务

一般情况下，事务必须在单个连接上完成。在分布式事务中（极少情况下），可以跨多连接/多实例:使用XA事务（两阶段提交，2PC）或类似协议，但是性能极差，复杂

- 事务内所有操作都是串行的，可用上下文控制进度
- 可从外部 Stmt 进行参数复制，创建事务模版
- 事务内 Prepare 同样需要 Stmt.Close，且必须在提交和回滚前
- 提交 Commit 和 回滚 Rollback 都会判断完成状态

```go
package main

import (
	"database/sql"
	"log"

	_ "github.com/mattn/go-sqlite3"
)

func main() {
	log.SetFlags(log.Lshortfile)

	db, err := sql.Open("sqlite3", "./test.db")
	if err != nil { log.Fatalln(err) }
	defer db.Close()

	tx, err := db.Begin()
	if err != nil { log.Fatalln(err) }

	// exec table
	execs := []func() error {
		// insert
		func() (err error) {
			q := `INSERT INTO user (id, name) VALUES (?, ?)`
			_, err = tx.Exec(q, 102, "u102")
			return
		},
		// select
		func() (err error) {
			var name string

			q := `SELECT (name) FROM user WHERE id = ?`
			err = tx.QueryRow(q, 102).Scan(&name)
			if err != nil { return err }

			println(name)
			return nil
		}
	}

	// exec, rollback
	for _, exec := range execs {
		if err := exec(); err != nil {
			if e := tx.Rollback(); e != nil { log.Fatalln(e) }
			log.Fatalln(err)
		}
	}
	// commit
	if e := tx.Commit(); e != nil && e != sql.ErrTxDone { log.Fatalln(e) }
}
```

事务单连接特性，下面的操作有问题

```go
tx, _ := db.Begin()
// 占用事务的连接
rows, _ := tx.Query("SELECT id FROM user")

for rows.Next() {
    var id int
    rows.Scan(&id)
    // 这里就会死锁！因为rows占用连接，无法执行其他查询
    tx.QueryRow("SELECT name FROM user WHERE id = ?", id).Scan(&name)
}
```

### scan & null

#### RawBytes

接收任何数据的字节数组，可用于未知类型字段

- 引用底层驱动 (sql/driver) 管理的内存
- 有效期在下次 Next、Scan、Close调用前
- 不能用于 Row.Scan

```go
type RawBytes []byte

for rows.Next() {
	var name sql.RawBytes
	rows.Scan(&name)
}
```

#### Null

标准库专门为常用类型提供了空值类型

```go
type NullString struct {
	String string
	Valid bool
}

type NullInt64 struct {
	Int64 int64
	Valid bool
}

var name sql.NullString

rows.Scan(&name)

if name.Valid {
	println(name.String)
}
```

#### Valuer, Scanner

基于两个接口实现 Scan 自定义编码和解码操作。比如压缩、加密等。空值类型就是实现了这两个接口

- Scanner: 对返回的数据进行处理
- Valuer: 对准备写入的数据进行处理

### context

添加上下文，避免操作阻塞

```go
package main

import (
	"database/sql"
	"log"
	"time"
	"context"

	_ "github.com/mattn/go-sqlite3"
)

var (
	db *sql.DB

	ctx = context.Background()
	timeout = time.Millisecond
)

func ping() {
	// 左边的ctx 「shadow」 了 全局变量 ctx
	ctx, cancel := context.WithTimeout(ctx, timeout)
	defer cancel()

	var name string
	err := db.QueryRowContext(ctx, `SELECT name FROM user`).Scan(&name)

	if err == context.DeadlineExceeded {
		println("timeout!")
	} else if err != nil {
		log.Println(err)
	} else {
		println(name)
	}
}

func main() {
	db, _ = sql.Open("sqlite3", "./test.db")

	ping()
}
```