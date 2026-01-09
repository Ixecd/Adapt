## etcd 记录

## 认证与授权
- 添加用户 root `etcdctl user add root:root`
- 给予root用户root权限(必须) `etcdctl user grant-role root root`
- 开启认证 `etcdctl auth enable`
之后每次使用etcdctl都需要通过 --user name:password 来指定用户，匿名用户无法访问

etcd server 之后收到请求的时候，在提交到Raft模块前，会从请求的上下文中获取当前的用户身份信息。如果未通过认证，那么在状态机应用put命令的时候，检查身份权限的时候发现是空，就会返回错误给client

鉴权模块收到一个通过root用户添加一个新用户alice:alice的命令时，它会使用 bcrpt 库的 blowfish 算法，基于明文密码、随机分配的salt、自定义的cost、迭代多次计算得到一个hash值，并将加密算法版本、salt值、cost、hash值组成一个字符串，作为加密后的密码

鉴权模块将用户名作为key，加密后的密码作为value，存储到boltdb的authUsers bucket里面，完成一个账号的创建

验证密码的过程就是 根据明文再算一次跟 boltdb中的密码对应

### 认证

身份验证这个过程开销机器昂贵，需要保证性能， simple token 和 jwt token

### Simple Token & JWT Token

**Simple Token**

核心原理就是当一个用户身份验证通过之后，生成一个随机的字符串值Token返回给client，并在内存中使用map存储用户和token之间的映射关系。每个Token都有expire，过期后需要再次验证身份

**JWT Token**

JWT 是 Json Web Token的缩写，基于JSON开放标准定义的一种 紧凑、独立的格式，用于在身份提供者和服务提供者间，传递被认证的用户身份信息。
由 Header、Payload、Signature三个对象组成，每个对象都是一个JSON结构体。

**Header**
```json
{
	"alg": "RS245",
	"typ": "JWT"
}
```

**Payload**
```json
{
	"username": name,
	"revision": revision,
	"exp":		time.Now().Add(t.ttl).Unix(),
}
```

**Signature**

其将header、payload使用base64 url编码，之后将编码后的字符串使用 . 连接在一起，最后使用 签名算法，比如RSA系列的私钥对其计算签名，输出结果就是Signature

也就是 `base64UrlEncode(header).base64UrlEncode(payload).signature`组成
### Cert认证
这是etcd的另外一种高性能、更安全的鉴权方案，x509证书认证

密码认证一般使用在client和server基于HTTP协议通信的内网场景中。当对安全有更高要求的时候，需要使用HTTPS协议加密通信数据，防止中间人工具和数据被篡改等安全风险。

HTTPS是利用非对称加密实现身份认证和密钥协商，因此使用HTTPS协议的时候，需要使用CA证书给client生成证书才能访问。

可以使用下面的命令来查看client证书的内容
```sh
openssl x509 -noout -text -in client.pem
```
证书中一般都含有证书版本、序列号、签名算法、签发者、有效性、主体名等信息，重点需要关注**主体名的中的CN字段**

在etcd中，如果使用了HTTPS协议并启用了client证书认证(`-client-cert-auth`)，它会取**CN**字段作为用户名

证书认证在稳定性、性能上都优于密码认证

稳定性上，它不存在Token过期、使用更加方便，避免了不少Token失效而触发的Bug

性能上，证书认证不需要像密码认证一样调用昂贵的密码认证操作

## 授权

开启鉴权之后，put请求命令在应用到状态机前，etcd还会发出对此请求的用户进行权限检查，判断其是否有权限操作请求的数据。

常用的权限控制方法有ACL（Access Control List）、ABAC（Attribute-based access control）、RBAC（Role—based access control），etcd使用的RBAC

**RBAC(基于角色权限的控制系统)**

其由，**User**, **Role**, **Permission** 三部分组成。
- User: 表示用户
- Role: 表示角色
- Permission: 表示具体权限明细，比如赋予 Role 对 key范围在[key, keyEnd]数据有什么权限，目前支持三种权限，分别是**READ**, **WRITE**, **READWRITE**

```bash
# 创建一个admin role
etcdctl role add admin --user root:root
# 分配一个可读写[hello,helly]范围数据的权限给admin role
etcdctl role grant-permission admin readwrite hello helly --user root:root
# 将用户alice和admin role关联起来，赋予admin权限给user
etcdctl user grant-role alice admin --user root:root
```
之后使用用户alice去put或者get hello命令时，鉴权模块会从 boltdb 查询alice用户对应的权限列表

有可能一个用户拥有成百上千个权限列表，etcd为了提升权限检查的性能，引入了区间树，检查用户操作的key是否在已经授权的区间，时间复杂度仅为 O(logN)

## Lease
etcd默认的Lease最小值是1s

## MVCC
从名字上理解，它是一个基于多版本技术实现的一种并发控制机制

常见的并发控制机制有 悲观锁、读写锁、互斥锁、两阶段锁等

**MVCC机制是基于多版本技术实现的一种乐观锁机制**

在MVCC数据库中，更新一个key-value数据的时候，不会去覆盖原来的数据，而是新增一个版本来存储新的数据，每个数据都有一个版本号，删除数据的时候，实际上也是新增一条带删除标识的数据记录

修改前的详细信息
![修改前的详细信息](./images/revision_1.png)

修改后的详细信息
![修改后的详细信息](./images/revision_2.png)

对比修改前后，可以发现：
- 修改后 revision + 1
- kvs中的 mod_vision 和 version 都 + 1， 这里 key 和 value 都经过了 base64 处理

`etcdctl get hello --rev=2 --endpoints=http://127.0.0.1:2379 --user root:root -w json | jq`

![get hello --rev=2](./images/revision_3.png)

对上面的信息需要进行一些解释，`header` 中的 `revision` 反映的「现在」，而 `kvs` 中的 `revision` 反映的是「过去」

- `create_revision` 表示这个key是在全局版本号为2的时候被创建的
- `mod_revision` 表示这个key最近一次被修改，是在全局版本号为2的时候

`--rev`这个选项既不是专门「创建版本」也不是找「修改版本」，而是找**快照版本**，本质上就是 <= mod_revision最大的那个

删除这个key之后，依然可以根据revision来找到相应的value，除非etcd执行了**压缩(Compaction)**操作时，带有旧Value的历史版本会真正从磁盘上被物理清理掉

![delete Hello](./images/revision_4.png)


### 整体架构
整个MVCC特性由treeIndex、boltdb组层

Apply模块通过MVCC模块来执行put请求，持久化key-value数据。MVCC模块将请求划分为两个类别，分别是读事务(ReadTxn)和写事务(WriteTxn)。读事务负责处理range请求，写事务负责put/delete请求。读写事务基于treeIndex、Backend(boltdb)提供的能力实现对key-value的增删改查功能

treeIndex模块基于内存版的 B-tree实现key索引管理，其保存了用户key与版本号(所有mod_revision)的映射关系等信息。不添加`--rev`选项时，treeIndex模块会根据用户输入的key，在内存中查找对应的版本号(最新的mod_revision)，然后根据版本号去Backend模块中查找对应的value

Backend模块负责etcd的key-value持久化存储，主要由ReadTx、BatchTx、Buffer组成，ReadTx定义了抽象的读事务接口，BatchTx在ReadTx之上定义了抽象的写事务接口，Buffer是数据缓冲区

etcd设计上支持多种Backend实现，目前实现的Backend是boltdb，其是一个基于B+tree实现的、支持事务的key-value嵌入式数据库

### treeIndex原理
treeIndex出现的原因就是v2版本的etcd直接会将新的value覆盖旧的value，无法支持保存key的历史版本，其可以提供稳定的Watch机制和实物隔离等能力

功能上，etcd支持范围查询，因此保存索引的数据结构也必须支持范围查询，哈希表不适合，B-tree支持范围查询

性能上，平衡二叉树内个节点只能容纳一个数据、树的高度比较高，B-tree每个节点可以容纳多个数据，树的高度更低、更扁平，涉及的查找次数更少，具有优越的增删改查性能

在一个**度**为 **d** 的B-tree中，节点保存的最大key数量为 **2d - 1**，最多的子节点数量是 **2d**，etcd treeIndex模块中，创建的最大度是32，也就是说内部节点最多可以保存63个key，最多可以有64个子节点

在TreeIndex中，每个节点的key是一个keyIndex结构，etcd就是通过它保存了用户 key 与 版本号的映射关系

```go
type keyIndex struct {
	key				[]byte		// 用户key名称
	modified 		revision	// 用户key最近一次修改的全局版本号，即mod_revision
	generations		[]generation// 用户key的多个版本号，即所有create_revision
}
type generation struct {
	version			int64		// 表示此key的修改次数
	created			revision	// 表示generation结构创建时的版本号
	revs			[]revision	// 每次修改key时的revision追加到此数组
}
type revision struct {
	main			int64		// 全局递增的主版本号，随put/txn/delete事务递增，一个事务内的key main版本号是一致的
	sub				int64		// 一个事务内的子版本号，从0开始随事务内put/delete操作递增
}
```
generation结构体中包含此key的修改次数、generation创建时的版本号、对此key的修改版本号记录列表

revision包含main和sub两个字段，main是全局递增的版本号，它是etcd逻辑时钟，随着put/txn/delete等事务递增。sub是一个事务内的子版本号，从0开始随事务内的put/delete操作递增

### MVCC更新key原理

在put写事务中，先从treeIndex查key的keyIndex索引信息，keyIndex中存储了key的创建版本号、修改的次数等信息，这些信息在事务中发挥着重要作用，会存储在boltdb的value中

1. 获取key的keyIndex信息(包含Ver，CreatedVer信息)
2. key:revision{txid, 0}, Value: KeyValue写入新的key-value到boltdb和buffer中
3. 创建/更新keyIndex，更新keyIndex到treeIndex
4. backend异步事务提交goroutine

boltdb的value是**mvccpb.KeyValue**结构体，它是由 key、value、 create_revision、 mod_revision、 lease组成。填充好boltdb的KeyValue结构体后，这时就可以通过Backend的写事务batchTx接口将 key{revision, 0}, value为 mvccpb.KeyValue 保存到boltdb的缓存中，并同步更新buffer

此时存储到boltdb中的key、value数据如下

```
	command						boltdb key				boltdb value/mvccpb.KeyValue
etcdctl put hello world1		key{2, 0}			value{key: base64(hello), value: base64(world1), create_revision: 1, mod_revision: 1, lease: 0}
```

**Version** 和 **ModRevision**
- **Version** 指的是修改的次数
- **ModRevision** 指的是根据revision(全局)来设定的

此时数据还未持久化，为了提升etcd的写吞吐量、性能，一般情况下（默认堆积的写事务数大于1万才在写事务结束时同步持久化），数据持久化由Backend的异步goroutine完成，通过事务批量提交，定时将boltdb页缓存中的脏数据提交到持久化存储磁盘中

### MVCC查询key原理
完成put hello为world操作后，这时需要通过etcdctl发起一个get hello操作，MVCC模块首先会创建一个读实物对象(TxnRead)，在etcd 3.4中Backend实现了ConcurrentReadTx，并发读特性

并发读特性的核心原理是创建读事务对象时，会全量拷贝当前写事务未提交的buffer数据，并发的读写事务不再阻塞在一个buffer资源锁上，实现了全并发读

先去treeIndex找key(keyIndex对象,匹配有效的generation,获取的是key{mod_revision, 0})，之后去buffer里找，未命中就去boltdb里找

指定版本号读取历史记录的实现方法: 如果后续再通过 put hello worldx 修改操作时，key hello对应的keyIndex结果如下所示， keyIndex.modified字段更新为<3,0>，generation的revision数组追加最新的版本号<3, 0>，ver修改为2。boltdb会插入一个新的key revision{3, 0}，指定版本号之后treeIndex模块会遍历generation内的历史版本号，返回小于等于mod_revision最大历史版本号

### MVCC删除key原理
etcd实现的是标记延期删除模式，原理与key更新类似

生成的boltdb key版本号为 key{x, 0, t}追加了删除标识(tombstone,简写t)，boltdb value变成只含用户key的KeyValue结构体。treeIndex模块也会给此key hello对应的keyIndex对象添加一个generation结构体，表示此索引对应的key被删除了

etcdctl delete hello 操作后的keyIndex结果如下面所示

![delete hello keyIndex](./images/revision_5.png)

boltdb 中的 key-value 数据如下图所示

![boltdb](./images/revision_6.png)

删除key时会生成events，Watch模块根据key的删除标识，会生成对应的Delete事件。重启etcd，遍历boltdb中的key构建treeIndex内存树时，需要知道哪些key是已经被删除的，并为对应的key索引生成tombstone标识

真正删除treeIndex中的索引对象、boltdb中的key是通过(compactor)组件异步完成

## Watch机制
高效获取数据变化通知

etcd的Watch特性是Kubernetes控制器的工作基础

![watch机制](./images/watch_1.png)

1. client获取事件的机制，etcd是使用轮训模式还是推送模式呢？两者各有什么优缺点？

2. 事件是如何存储的？会保留多长时间？watch命令中的版本号具体有什么作用呢？

3. 当client和server端出现短暂网络波动等异常因素后，导致事件堆积时，etcd是如何根据变化的key快速找到监听它的watcher呢？

4. 如果创建了上万个watcher监听key变化，当server端收到一个写请求后，etcd是如何根据变化的key快速找到监听它的watcher呢？

### 轮训 & 流式推送
这两种机制etcd都有使用

etcd v3中，使用基于HTTP/2的gRPC协议，双向流的Watch API设计，实现了连接多路复用

在HTTP/2协议中，HTTP消息会被分解成独立的帧(Frame)，交错发送，帧是最小的数据单位。每个帧会标识属于哪个流(Stream)，留由多个数据帧组成，每个流拥有一个唯一的ID，这个数据流对应一个请求或响应包。HTTP/2可基于帧的流ID将并行、交错发送的帧重新组装成完整的消息。

基于HTTP/2的多路复用机制，实现一个client/TCP连接支持多gRPC Stream，一个gRPC Stream又支持多个watcher。事件通知模式从v2的client轮训优化成server流式推送，极大降低了server端socket、内存等资源

在clientv3库中，Watch特性被被抽象成Watch、Close、RequeestProgress三个简单API提供给开发者使用，屏蔽了client与gRPC WatchServer交互的复杂细节，实现了一个client支持多个gRPC Stream，一个gRPC Stream支持多个watcher，显著降低了开发复杂度

当watch连接的节点故障，clientv3库支持自动重连到健康节点，并使用之前已接收的最大版本号创建新的watcher，避免旧事件回放等

### 滑动窗口 & MVCC
上面问题2的本质是**历史版本存储**，etcd经历了从 滑动窗口 到 MVCC 机制的演变

etcd v2是使用一个简单的环形数组来存储历史事件版本，当key被修改后，相关事件就会被添加到数组中来。若超过eventQueue的容量，则淘汰最旧的事件。v2中eventQueue的容量固定是1000，不会占用大量内存导致OOM
```go
type EventHistory struct {
	Queue			eventQueue
	StartIndex		uint64
	LastIndex		uint64
	Rev				sync.RWMutex
}
```
其缺陷是显而易见的，固定的事件窗口只能保存有线的历史事件版本，是不可靠的。当写请求较多的时候、client与server网络出现波动等异常时，很容易导致事件丢失，client不得不触发大量的expensive查询操作，以获取最新的数据以及版本号，才能持续监听数据

对于重度依赖Watch机制的Kubernetes而言，无法接受，因为会导致控制器等组件频繁的发起expensive List Pod等资源操作，导致APIServer/etcd出现高负载、OOM等，对稳定性会造成严重影响

MVCC机制就是为了解决v2 watch机制不可靠诞生的。v3将一个key的历史修改版本保存在boltdb里面。boltdb是一个基于磁盘文件的持久化存储，不会丢失数据

revision是etcd的逻辑时钟，当client因为网络等异常出现连接闪断后，通过revison就可以从server端的boltdb中获取错过的历史事件，而无需全量同步，其是etcd watch机制数据增量同步的核心

### 可靠的事件推送机制

当通过etcdctl或API发起一个watch key请求的时候，etcd的gRPCWatchServer收到watch请求后，会创建一个serverWatchStream，它负责接收client的gRPC Stream的create/cancel watcher请求(recvLoop goroutine)，并将从MVCC模块接收到的Watch时间转发给client(sendLoop goroutine)

当serverWatchStream收到create watcher请求后，serverWatchstream会调用MVCC模块的WatchStream子模块分配一个watcher id，并将watcher注册到MVCC的WatchableKV模块

在etcd启动时，watchableKV模块会运行syncWatchersLoop和syncVictimsLoop goroutine，分别负责不同场景下的事件推送

etcd在面对各类异常，实现可靠事件推送的机制是 **复杂度管理，问题拆分**

etcd根据不同场景，对问题进行了分解，将watcher按场景分类，实现了轻重分离、低耦合

**synced watcher** 表示此类watcher监听的数据都已经同步完成，在等待新的变更


