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

**示例**

如果不指定endpoints，其默认值就是 http://127.0.0.1:2379，其参数顺序也是有要求的，global flags **必须要放在子命令前面**，下面命令能正常执行，是因为 etcdctl 基于Cobra库，全局选项**理论上**必须放在子命令（get/put/txn）之前，但在get/put/del等简单子命令中，Cobra解释器对persistent flags有一定「宽容度」

`etcdctl get hello --rev=2 --endpoints=http://127.0.0.1:2379 --user root:root -w json | jq`

![get hello --rev=2](./images/revision_3.png)

对上面的信息需要进行一些解释，`header` 中的 `revision` 反映的「现在」，而 `kvs` 中的 `revision` 反映的是「过去」

- `create_revision` 表示这个key是在全局版本号为2的时候被创建的
- `mod_revision` 表示这个key最近一次被修改，是在全局版本号为2的时候

`--rev`这个选项既不是专门「创建版本」也不是找「修改版本」，而是找**快照版本**，本质上就是 <= mod_revision最大的那个

删除这个key之后，依然可以根据revision来找到相应的value，除非etcd执行了**压缩(Compaction)**操作时，带有旧Value的历史版本会真正从磁盘上被物理清理掉

![delete Hello](./images/revision_4.png)

强烈推荐使用环境变量彻底摆脱顺序烦恼
```zsh
export ETCDCTL_ENDPOINTS=http://127.0.0.1:2379
export ETCDCTL_USER=root
export ETCDCTL_PASSWORD=root
etcdctl get hello --rev=2 -w json | jq
```

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

### 轮询 & 流式推送
轮询v2，流式推送v3

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

**unsynced watcher** 表示此类watcher监听的数据还未同步完成，落后于当前最新数据变更，正在追赶

### 最新时间推送机制
当etcd收到一个写请求，key-value发生变化时，处于syncedGroup中的watcher，是如何获取到最新变化事件并推送给client的呢？

当创建完成watcher后，执行put hello修改操作时，请求会经过 KVServer、Raft模块后Apply到状态机时，在MVCC的put事务中，它会将本次修改后的mvccpb.KeyValue保存到一个changes数组中，put事务结束的时候，会将KeyValue转换成Event事件，回调watchableStore.notify函数，notify会匹配出监听此key并处于synced watcherGroup中的watcher，同时事件中的版本号要大于等于watcher监听的最小版本号，才能将事件发送到此watcher，watcherStream.notify会把event打包成WatchResponse，最终push到watcherStream的sendCh channel中，serverWatchStream的sendLoop goroutine监听到channel消息后，读出消息立即推送给client。至此，完成一个最新修改事件推送

etcd MVCC层用于构建watch事件的核心逻辑如下
```go
// changes 是一组内部表示的KeyValue变化记录
// evs 是最终要提供给 watch 客户端的事件数组
evs := make([]mvccpb.Event, len(changes))
// 循环，将每个 changes[i] 转换成 evs[i]
for i, change := range changes {
	// 将 KeyValue 放入 event 中
	evs[i].Kv = &changes[i]
	// 核心逻辑，如果 CreateRevision 为0，则视为删除
	if change.CreateRevision == 0 {
		evs[i].Type = mvccpb.DELETE
		// 不会真正删除，还要写条数据，revison改成当前 rev
		evs[i].Kv.ModRevision = rev
	} else { // 不是 DELETE 就是 PUT（合并创建与修改）
		evs[i].Type = mvccpb.PUT
	}
}
// 向 watcher 推送事件
// tw 是 wathcer 对象（订阅者） （非 gRPC watcher，是内部watch触发器的 watcher）， .s 是 watcher 持有的 store 对象，类型是 *watchableStore， notify是 watchableStore 的方法
tw.s.notify(rev, evs)
```

watch事件channel的buffer容量默认是1024，buffer满了事件会丢失吗？

### 异常场景重试机制
如果出现channel buffer满了，etcd为了保证Watch事件的高可靠性，并不会丢弃它，而是将此watcher从synced watcherGroup中删除，然后将此watcher和事件列表保存到一个名为受害者victim的watcherBatch结构中，通过**异步机制重试**保证事件的可靠性，notify操作它是在修改事务结束时同步调用的，必须是轻量级、高性能、无阻塞的，负责会严重影响集群写性能

WatchableKV模块会启动两个异步goroutine，其中一个是syncVictimsLoop，正是它负责slower watcher的堆积的事件推送

其工作原理就是，遍历victim watcherBatch数据结构，尝试将堆积的事件再次推送到watcher的接收channel中。如果推送失败，则再加入到victim watcherBatch数据结构中等待下次重试

如果推送成功，watcher监听的最小版本号(minRev)小于等于server当前版本号(currentRev)，说明可能还有历史事件未推送，需加入到unsynced watcherGroup中，由历史事件推送机制，推送minRev到currentRev之间的事件

如果watcher的最小版本号大于server当前版本号，则加入到synced watcher集合中，进入最新事件通知机制

### 历史事件推送机制
WatchableKV模块的另外一个goroutine，syncWatchersLoop，正是负责unsynced watcherGroup中的watcher历史事件推送

syncWatchersLoop，会遍历处于unsynced watcherGroup中的每个watcher，为了优化性能，它会选择一批unsynced watcher批量同步，找出这一批unsynced watcher中监听的最小版本号

boltdb的key是按照版本号存储的，可以通过指定查询的key范围的最小版本号作为开始区间，当前server最大版本号作为结束区间，遍历boltdb获得所有历史数据

将KeyValue结构转换成事件，匹配出监听过事件中key的watcher后，将事件发送给对应的watcher事件接收channel即可。发送完成之后，watcher从unsynced watcherGroup中移除、添加到synced watcherGroup中

如果watcher监听的版本号已经小于当前etcd server压缩的版本号，历史变更数据就可能已经丢失，因此etcd server会返回ErrCompacted错误给client（想看的部分旧历史没有了，重新先使用GET获取当前最新的revision，再watch）

### 高效的事件匹配
一个个遍历watcher是最简单的方法，但性能较差，在watcher数量较多的场景下，会导致性能出现瓶颈。更何况etcd是在执行一个写事务结束时，同步触发事件通知流程的，若匹配watcher开销较大，将严重印象etcd性能

~~etcd使用map记录监听单个key的watcher，但是watch特性不仅仅是可以监听单key，它还可以指定监听key范围、key前缀，因此etcd还使用了如下区间数~~
![事件匹配区间树](./images/watch_2.png)

key空间被简化成了字母 a - z（实际上etcd的key是[]byte，按字典序比较，空间无限大，但是原理一样）
- 每个节点包含三个东西
	- 一个区间 `[low, high)`
	- 一个watcherSet: 所有精准监听这个区间的客户端的watcher集合
- 树是层次分割的
	- 根节点覆盖**最大可能范围**
	- 子节点范围是父节点的**严格子集**，并且子节点区间**互不重叠**，他们的并集等于父节点区间
	- 分割方式类似二分
- 这是一种**动态段树思想在区间上的实现**，或者叫**中心点区间树**。树会保持大致平衡（高度O(LogU)， U 是key的 宇宙大小（key可能取值的全集范围），或实际端点数）

当收到创建watcher请求的时候，会把watcher监听的key范围插入到上面的区间树中，区间的值保存了监听同样key范围的watcher集合/watcherSet

当产生一个事件时，etcd首先需要从map查找是否有watcher监听了单个key，其次还需要从区间树找出与此key相交的所有区间，然后从区间的值获取监听的watcher集合

区间树支持快速查找一个key是否在某个区间内，时间复杂度为O(LogN)，因此etcd基于map和区间数实现了watcher与事件快速匹配，具备良好的扩展性

其实上面的美好设计，并内有真正被采纳（etcd从v3到现在并没有使用区间树来管理范围watcher）

实际上etcd的notify中使用了for range来遍历synced watcher（synced watcher数量通常几十到几百，k8s一个etcd实例服务多个namespace，但总watcher可控）

如果 watcher 爆炸，那么官方推荐：使用多个etcd集群隔离、客户端聚合watcher、避免宽范围前缀

## 事务
etcd v2的时候，etcd提供了CAS（Compare and swap），然而其只支持单key，不支持多key，因此无法满足类似转账场景的需求

etcd v3为了解决多key的原子操作问题，提供了全新迷你事务API，同时基于MVCC版本号，实现了各种隔离级别的事务

`clientv3.Txn(ctx).If(cmp1, cmp2, ...).Then(op1, op2. ...,).Else(op1, op2, ...)`

事务API由If语句、Then语句、Else语句组成

基本原理是，在If语句中，可以添加一系列的条件表达式，若条件表达式全部通过检查，则执行Then语句的get/put/delete操作，否则执行Else中的get/put/delete等操作

If语句中，支持检查项有如下几条
1. key的最近一次修改版本号mod_revision，简称mod
2. key的创建版本号create_revision，简称create
3. key的修改次数version
4. key的value值

![事务API](./images/txn_1.png)

当通过client发起一个txn转账事务操作时，通过gRPC KV Server、Raft模块处理后，在Apply模块执行此事务的时候，它首先对你的事务的If语句进行检查，也就是ApplyComares操作，如果通过此操作，则执行ApplyTxn/Then语句，否则执行ApplyTxn/Else语句

在执行过程中，会根据事务是否只读、可写，通过MVCC层的读写事务对象，执行事务中的get/put/delete各操作

### 事务ACID特性
ACID是衡量事务的四个特性，由原子性（Atomicity）、一致性（Consistency）、隔离性（Isolation）、持久性（Durability）组成

### 原子性与持久性
原子性（Atomicity）是指在一个事务中，所有请求要么同时成功，要么同时失败

持久性（Durability）是指事务一旦提交，其所做的修改会永久保存在数据库中

![原子性与持久性](./images/txn_2.png)

**T1时间点**

Alice扣款100元完成时，Bob账号资金还未成功增加时突然发生了crash

此时MVCC写事务持有boltdb写锁，仅是将修改提交到了内存中，保证幂等性、防止日志条目重复执行的一致性索引consistent index也并未更新。同时，负责boltdb事务提交的goroutine因无法持有写锁，也并未将事务提交到持久化存储中

T1时间点发生crash异常后，事务并未成功执行和持久化任意数据到磁盘上。在节点重启时，etcd server会重放WAL中的已提交的日志条目，再次执行以上转账事务

**T2时间点**

MVCC写事务完成转账，server返回给client转账成功后，boltdb的事务提交goroutine，批量将事务持久化到磁盘中时发生了crash

一致性索引consistent index字段值是和key-value数据在一个boltdb事务里同时持久化到磁盘中的。如果在boltdb事务提交过程中发生了crash，简单情况是consistent index和key-value数据（一次事务中多个KV对应一个consistent index）都更新失败。当节点重启，etcd server重放WAL中已提交日志条目时，同样会再次应用转账事务到状态机中，因此事务的原子性和持久性依然能得到保证

### 一致性
其实在不同场景下，其含义是不一样的

1. 分布式系统中多副本数据一致性，是指各个副本之间的数据是否一致，比如Redis的主备是异步复制的，那么它的一致性就是最终一致性的
2. CAP原理中的一致性是指可线性化。核心原理是最燃整个系统是由多副本组成，但是通过线性化能力支持，对client而言就如一个副本，应用程序无需关心系统有多少个副本
3. 一致性哈希，是一种分布式系统中的数据分片算法，具备良好的分散性、平衡性
4. 事务中一致性，是指事务变更前后，数据库必须满足若干恒等条件的状态约束，一致性往往是由数据库和业务程序两方面来保障的

![一致性](./images/txn_3.png)

为了确保事务的一致性，一方面，业务程序在转账逻辑里面，需检查转账者资产大于等于转账金额。在提交事务时，通过账号资产的版本号，确保双方账号资产未被其他事务修改。如果双方账号资产被其他事务修改，账号资产版本号会检查失败，这时业务可以通过获取最新的资产和版本号，发起新的转账事务流程解决

另一方米，etcd会通过WAL日志和consistent index、boltdb事务特性，去确保事务的原子性，因此不会有部分成功失败的操作，导致资金凭空消失、新增

### 隔离性
常见的事务隔离级别有如下四种
1. **未提交读（Read Uncommitted）**: 一个client能读取到未提交的事务。比如转账事务过程中，Alice账号资金扣除后，Bob账号上资金还未增加，这时如果其他client读取到这种中间状态，它会发现系统总金额钱减少了，破坏了事务一致性的约束
2. **已提交读（Read Committed）**: 指只能读取到已经提交的事务数据，但存在不可重复读的问题。比如事务开始时，你读取了Alice和Bob资金，这时其他事务修改Alice和Bob账号上的资金，你在事务中再次读取时会读取到最新资金，导致两次读取结果不一样。
3. **可重复读（Repeated Read）**: 它是指在一个事务中，同一个读操作get Alice/Bob在事务的任意时刻都能得到同样的结果，其他修改事务提交后也不会影响你本事务所看到的结果。
4. **串行化（Serializable）**: 最高级别的事务隔离，读写相互阻塞，通过牺牲并发能力、串行化来解决事务并发更新过程中的隔离问题。对于串行化我要和你特别补充一点，很多人认为它都是通过读写锁，来实现事务一个个串行提交的，其实这只是在基于锁的并发控制数据库系统实现而已。**为了优化性能，在基于MVCC机制实现的各个数据库系统中，提供了一个名为“可串行化的快照隔离”级别，相比悲观锁而言，它是一种乐观并发控制，通过快照技术实现的类似串行化的效果，事务提交时能检查是否冲突**

通过MVCC快照读，或者参考etcd的事务框架STM实现，其在事务中维护一个读缓存，优先从读缓存中查找，不存在则从etcd查询并更新到缓存中，确保了可重复读

**串行化快照隔离**

指在事务刚开始的时候，首先获取etcd当前的版本号rev，事务中后续发出的读请求都带上这个版本号rev，告诉etcd你需要获取那个时间点的快照数据，etcd的MVCC机制就能确保事务中能读取到同一时刻的数据

同时，它还要确保事务提交时，读写的数据都是最新的，未被其他人修改，业绩就是要增加冲突检测机制（MVCC的版本号），事务提交出现冲突的时候依赖client重试解决，安全的实现多key原子更新

![串行化快照隔离](./images/txn_4.png)

事务A，Alice往Bob转账100，事务B，Mike向Bob转账100，两个事务同时发起转账操作

一开始，Mike版本号（mod_revision）是4，Bob版本号是3，Alice版本号是2，资产各自200。为了防止并发写事务冲突，etcd在一个写事务开始时，会独占一个MVCC读写锁

事务A和B会先去etcd查询当前Alice和Bob的资产版本号，用于在事务提交时做冲突检测。在事务A和B查询后，事务B加入先获得MVCC写锁并完成转账事务，Mike和Bob账号资产分别为100,300，版本号都是5（图里是5，我这里是12）

事务B完成后，事务A获得写锁，开始执行事务

事务A，期望的Alice版本号应该为2，Bob为3，但是实际上是5，那就重新获取Alice和Bob的资产版本号，再次执行事务，直到成功为止

![eg](./images/txn_5.png)

输出不够美观，搞个终端函数

```zsh
etcdtxn() {
  local raw
  raw=$(etcdctl txn -i -w json 2>&1)

  # 提取最后以 { 开头的纯 JSON 行（忽略所有提示文本）
  local json
  json=$(echo "$raw" | grep -E '^\{' | tail -n 1)

  if [ -n "$json" ]; then
    echo "$json" | jq -r '
      "Succeeded: \(.succeeded)",
      "Revision: \(.header.revision)",
      if .succeeded then
        if (.responses | length) > 0 then
          .responses[]
          | .Response.ResponseRange.kvs // empty
          | .[]
          | "\(.key | @base64d): \(.value | @base64d) (mod: \(.mod_revision), ver: \(.version))"
        else
          "No results in success branch (e.g., only put/del)"
        end
      else
        "Transaction failed (check compares)"
      end
    '
  else
    # 万一没提取到 JSON，直接显示原始输出（极少发生）
    echo "=== No JSON detected (possible error) ==="
    echo "$raw"
    echo "=== end raw ==="
  fi
}
```
美化后的效果图
![eg](./images/txn_6.png)

事务A完成后，B事务查找Compares会失败

![eg](./images/txn_7.png)

重新查找得到mod_revision为12，然后直接用12来执行

![eg](./images/txn_8.png)

## boltdb
etcd数据存储在boltdb上，boltdb是一个用Go语言编写的，基于B+树实现的，一个简单的、嵌入式的、持久化的KV数据库

### boltdb磁盘布局
boltdb文件指的是在etcd数据目录下的member/snap/db的文件，etcd的key-value、lease、meta、member、cluster、auth等所有数据存储在里面。etcd启动的时候，会通过mmap机制将db文件映射到内存，后续可从内存中快速读取文件中的数据。写请求通过fwrite和fdatasync来写入、持久化到磁盘中

![boltdb](./images/boltdb_1.png)

文件的内容由若干个page组成，一般情况下page size为4KB

page按照功能可分为元数据页（meta page）、B+ tree索引节点页（branch page）、B+ tree叶子节点（leaf page）、空闲页管理页（freelist page）、空闲页（free page）

文件最开头的两个page是固定的db元数据meta page，空闲页管理页记录了db中哪些页是空闲的、可使用的。索引节点页保存了B+ tree的内部节点，他们记录了key值，叶子节点页记录了B+ tree中的key-value和bucket数据

boltdb逻辑上是通过B+ tree来管理branch/leaf page，实现快速查找、写入key-value数据

### boltdb API
boltdb提供了非常简单的API给上层业务使用，当我们执行一个put hello为world命令时，boltdb实际写入的key是版本号，value为mvccpb.KeyValue结构体

假设往key bucket写入一个key为r94，value为world的字符串，其核心代码如下:
```go
// 打开boltdb文件，获取db对象
db,err := bolt.Open("db"， 0600， nil)
if err != nil {
   log.Fatal(err)
}
defer db.Close()
// 参数true表示创建一个写事务，false读事务
tx,err := db.Begin(true)
if err != nil {
   return err
}
defer tx.Rollback()
// 使用事务对象创建key bucket
b,err := tx.CreatebucketIfNotExists([]byte("key"))
if err != nil {
   return err
}
// 使用bucket对象更新一个key
if err := b.Put([]byte("r94"),[]byte("world")); err != nil {
   return err
}
// 提交事务
if err := tx.Commit(); err != nil {
   return err
}
```

### 核心数据结构介绍
boltdb整个文件是由一个个page组成。最开头的两个page描述db元数据信息，而它正是在client调用boltdb Open API时被填充的

boltdb本身自带了一个工具bbolt，它可以按页打印出db文件的十六机制内容

`go install go.etcd.io/bbolt/cmd/bbolt@latest`

`bbolt dump ./infra1.etcd/member/snap/db 0`

相关结果
![boltdb](./images/boltdb_2.png)

解释说明
![boltdb](./images/boltdb_3.png)

### page磁盘页结构
其由页ID（id）、页类型（flags）、数量（count）、溢出页数量（overflow）、页面数据起始位置（ptr）字段组成

页类型目前有如下四种:
1. 0x01: branch page
2. 0x02: leaf page
3. 0x04: meta page
4. 0x10: freelist page
数量字段仅在页类型为leaf和branch时生效，溢出页数量是指当前页面数据放不下的时候，需要向后再申请overflow个连续页面使用，页面数据起始位置指向page的载体数据，比如meta page、branch/leaf等page的内容

### meta page数据结构
前两页是固定存储db元数据的页（meta page），其由boltdb的文件标识（magic）、版本号（version）、页大小（pagesize）、boltdb的根bucket信息（root bucket）、freelist页面ID（freelist）、总的页面数量（pgid）、上一次写事务（txid）、校验码（checksum）组成

### bucket数据结构
可以使用bbolt buckets命令，输出一个db文件的bucket列表。执行完此命令后，可以看到auth、lease、meta等熟悉的bucket，它们都是etcd默认创建的

![boltdb](./images/boltdb_4.png)

meta page中，有一个名为root、类型bucket的重要数据结构，由root和sequence两个字段组成，root标识该bucket根节点的page id

meta page中的bucket.root字段，存储的是db的root bucket页面信息，上面看到的key、lease、auth等bucket都是root bucket的子bucket

```go
type bucket struct {
	root pgid // 根节点的page id
	sequence uint64 // 序列号
}
```

![bucket](./images/boltdb_5.png)

meta page十六进制图中，第三行就是描述root bucket信息，其指向的page id为4，从上图可以知道是 leaf page，当bucket比较少时，子bucket数据可直接从meta page里指向的leaf page中找到

### leaf page
leaf page的磁盘布局如下图所示，前半部分是leafPageElement数组，后半部分是key-value数据

![leaf page](./images/boltdb_6.png)

leafPageElement包含leaf page的类型flags，通过它可以区分存储的是bucket名称还是key-value数据

当flag是bucketLeafFlag（0x01）时，表示存储的是bucket数据，否则存储的是key-value数据，leafPageElement还含有key-value的读取变异量，key-value大小，根据偏移量和key-value大小，就可以方便地从leaf page中解析出所有key-value对

当存储的是bucket数据的时候，key是bucket的名称，value是bucket结构信息。bucket结构信息含有root page信息，通过root page（基于B+ tree查找算法），可以快速找到存储在这个bucket下面的key-value数据所在页面

boltdb还实现了inline bucket，在满足一些条件限制的情况下，可以将小的子bucket内嵌在它的父叶子结点上，友好的支持了大量小bucket

### branch page
boltdb采用了B+ Tree来高效管理所有bucket和key-value数据，因此它可以支持大量的bucket和key-value，只不过B+ tree的根节点不再直接指向leaf page，而是branch page索引节点页。branch page flags为0x01

![branch page](./images/boltdb_7.png)

branchPageElement包含key的读取偏移量、key大小、子节点的page id，根据偏移量和key大小，就可以方便的从branch page中解析出所有key，然后二分搜索匹配key，获取其子节点page id，递归搜索，直到从bucketLeafFlag类型的leaf page中找到目的bucket name

boltdb在内存中使用了一个名为 node 的数据结构，来保存page反序列化的结果
```go
func (n *node) read(p *page) {
   n.pgid = p.id
   n.isLeaf = ((p.flags & leafPageFlag) != 0)
   n.inodes = make(inodes, int(p.count))

   for i := 0; i < int(p.count); i++ {
      inode := &n.inodes[i]
      if n.isLeaf {
         elem := p.leafPageElement(uint16(i))
         inode.flags = elem.flags
         inode.key = elem.key()
         inode.value = elem.value()
      } else {
         elem := p.branchPageElement(uint16(i))
         inode.pgid = elem.pgid
         inode.key = elem.key()
      }
   }
```

### freelist
boltdb通过meta page中的freelist来管理页面的分配，freelist page中记录了哪些页是空闲的。当在boltdb中删除大量数据时，其对应的page就会被释放，页id存储到freelist所指向的空闲页中，写数据的时候，可以直接从空闲页中申请页面使用

之前的图中 第四行 就是描述freelist信息，page id为3，通过bbolt page命令查看3号page内容，它会记录空闲页（4，5）相关信息

![freelist](./images/boltdb_8.png)

freelist page存储结构如下所示，pageflags为0x10，表示freelist类型的页，ptr指向空闲页id数组，其实在boltdb中支持通过多种数据结构（数组和hashmap）来管理free page，这里是数组

![freelist](./images/boltdb_9.png)

### Open原理
boltdb Open API首先会打开db文件并对其增加文件锁，目的是防止其他进程也以读写模式打开它后，操作meta和fre page，导致db文件损坏

boltdb会通过mmap将db文件映射到内存中，并读取两个meta page到db对象实例中，然后校验meta page的magic、version、checksum是否有效，若两个meta page都无效，那么db文件就出现了损坏，导致异常退出

### Put原理

根据我们上面介绍的bucket的核心原理，它首先是根据meta page中记录root bucket的root page，按照B+ tree的查找算法，从root page递归搜索到对应的叶子节点page面，返回key名称、leaf类型

如果leaf类型为bucketLeafFlag，且key相等，那么说明已经创建过，不允许bucket重复创建，结束请求。否则往B+ tree中添加一个flag为bucketLeafFlag的key，key名称为bucket name，value为bucket的结构

创建完bucket后，就可以通过bucket的Put API发起一个Put请求更新数据。它的核心原理跟bucket类似，根据子bucket的root page，从root page递归搜索此key到leaf page，如果没有找到，则在返回的位置处插入新key和value

boltdb在内存中通过node数据结构来存储page磁盘页内容，它记录了key-value数据、page id、parent及children的node、B+ tree是否需要进行重平衡和分裂操作等信息。因此，当我们执行完一个put请求时，它只是将值更新到boltdb的内存node数据结构里，并未持久化到磁盘中。

### 事务提交原理

当代码执行tx.Commit API时，boltdb才会将上面保存的node内存数据结构中的数据，持久化到boltdb中

![txn](./images/boltdb_10.png)

插入了一个新的元素在B+ tree的叶子节点，它可能已不满足B+ tree的特性，因此事务提交时，第一步首先要调整B+ tree，进行重平衡、分裂操作，使其满足B+ tree树的特性。

在重平衡、分裂过程中可能会申请、释放free page，freelist所管理的free page也发生了变化。因此事务提交的第二步，就是持久化freelist。

注意，在etcd v3.4.9中，为了优化写性能等，freelist持久化功能是关闭的。etcd启动获取boltdb db对象的时候，boltdb会遍历所有page，构建空闲页列表。

事务提交的第三步就是将client更新操作产生的dirty page通过fdatasync系统调用，持久化存储到磁盘中。

最后，在执行写事务过程中，meta page的txid、freelist等字段会发生变化，因此事务的最后一步就是持久化meta page。

**事务提交过程中若持久化key-value数据到磁盘成功了，此时突然掉电，元数据还未持久化到磁盘，那么db文件会损坏吗？数据会丢失吗？ 为什么boltdb有两个meta page呢？**

1. boltbd采用的是 Copy-On-Write(COW)机制，写数据不会覆盖原来的，新找一块新的空闲页写入。如果写元数据meta还没开始或没写完掉电时，磁盘上确实会多一些新的KV数据页，但旧的Meta Page依然指向旧的 B+ Tree根节点；当数据库重启时，会读取旧的MetaPage
2. 两个Meta Page就是为了应对最极端的风险: 写 Meta Page那一瞬间掉电。BoltDB在文件头的第0页和第1页各放了一个Meta Page，事务ID为N是写0，N+1时写1，每个Meta Page都有一个校验位。当启动时，boltdb会同时读取这两个Meta Page，如果都有效，那就选择 Transaction ID 更大的那个，如果其中一个因为掉电导致写入受损，会自动回滚使用另一个**旧但完整的Meta Page**

## Compact

etcd中的每一次更新、删除key操作，treeIndex的keyIndex索引中都会追加一个版本号，在boltdb中会生成一个新版本boltdb key和value。也就是随着不停更新、删除，etcd进程内存占用和db文件就会越来越大。很显然，这会导致etcd OOM和db大小增长到最大配额，最终不可写

### 整体架构

![compact](./images/compact_1.png)

首先可以通过client API发起人工的压缩（Compact）操作，也可以配置自动压缩策略。在自动压缩策略中，可以根据业务场景选择合适的压缩模式。目前etcd支持两种压缩模式，分别是时间周期性压缩和版本号压缩

当通过API发起一个Compact请求后，KV Server收到Compact请求提交到Raft模块处理，在Raft模块中提交后，Apply模块就会通过MVCC模块的Compact接口执行此压缩任务

Compact接口首先会更新当前server已压缩的版本号，并将耗时昂贵的压缩任务保存到FIFO队列中异步执行。压缩任务执行时，它首先会压缩treeIndex模块中的keyIndex索引，其次会遍历boltdb中的key，删除已废弃的key

### 压缩特性初体验
在使用etcd过程中，当遇到「etcdserver: mvcc: database space exceeded」错误时，如果你未开启压缩策略导致db大小达到配额，这时可以使用etcdctl compact命令来主动触发压缩操作，回收历史版本

```sh
# 获取etcd当前版本号revision
rev = $(etcdctl endpoint status --write-out="json" | egrep -o '"revision":[0-9]*' | egrep -o '[0-9].*')
echo $rev

# 执行压缩操作，指定压缩的版本号为当前版本号
etcdctl compact $rev

# 压缩一个☝🏻已经压缩的版本号
etcdctl compact $rev

# 压缩一个比当前版本号大的版本号
etcdctl compact $((rev + 1))
```

![compact](./images/compact_2.png)

如果压缩命令传递的版本号小于等于当前etcd server记录的压缩版本号，etcd server会返回已压缩错误「mvcc: required revision has been compacted」给client。如果版本号大于当前etcd server最新的版本号，etcd server则会返回一个未来版本号错误给client「mvcc: required revision is a future revision」

这里压缩的本质是**回收历史版本**，目标对象仅是**历史版本**，不包括一个key-value数据的最新版本，因此可以放心执行压缩命令，不会删除最新的数据。但是Watch特性中的历史版本数据同步，依赖于MVCC中是否还保存了相关数据，所以不建议每次简单粗暴地回收所有历史版本

在生产环境中，建议精细化的控制历史版本数

1. 使用etcd server自带的自动压缩机制，根据业务场景，配置合适的压缩策略即可

2. 基于etcd的Compact API，在业务逻辑代码中、或定时任务中主动触发压缩操作，但是dev需要确保发起Compact操作的程序高可用，压缩的频率、保留的历史版本在合理范围内，并最终能使etcd的 db 大小保持平稳

建议使用etcd自带的压缩机制，它支持两种模式，分别是按时间周期性压缩和保留版本号的压缩，配置相应策略后，etcd节点会自动化的发起Compact操作

### 周期性压缩

etcd server 提供了配置压缩模式和保留时间的参数
```sh
--auto-compaction-retention '0'
Auto compaction retention length. 0 means disable auto Compaction.
--auto-compaction-mode 'periodic'
Interpret 'auto-Compaction-retention' one of: periodic|revision.
```

auto-compaction-mode为periodic时，它表示启用时间周期性压缩，auto-compaction-retention为保留的时间的周期，比如1h

auto-compaction-mode为revision时，它表示启用版本号压缩模式，auto-compaction-retention为保留的历史版本号数，比如10000

如果 etcd server 的 auto-compaction-retention为「0」，将关闭自动压缩策略

etcd server启动后，根据配置的模式periodic，会创建periodic Compactor，它会异步的获取、记录过去一段时间的版本号。periodic Compactor组件获取设置的压缩间隔参数1h，并将其划分成10个区间，也就是每个区间6分钟。每隔6分钟，它会通过etcd MVCC模块的接口获取当前的server版本号，追加到rev数组中

因为只需要保留过去1小时的历史版本，periodic Compactor组件会通过当前时间减去上一次成功执行Compact操作的时间，如果间隔大于1小时，它就会取出rev数组的首元素，通过etcd server的Compact接口，发起压缩操作

### 版本号压缩

当请求比较多，可能产生比较多的历史版本导致db增长时，或者不确定配置periodic周期为多少才是最佳的时候，可以通过设置压缩模式为revision，指定保留的历史版本号数。如果希望etcd尽量只保存1万个历史版本，那么可以指定compaction-mode为revision，auto-compaction-retention为10000

etcd启动后会根据压缩模式revision，创建revision Compactor。revision Compactor会根据设置的保留版本号数，每隔5分钟定时获取当前server的最大版本号，减去想保留的历史版本数，然后通过etcd server的Compact接口发起压缩操作即可

```go
# 获取当前版本号，减去保留的版本号数
rev := rc.rg.Rev() - rc.retention
# 调用server的Compact接口压缩
_，err := rc.c.Compact(rc.ctx，&pb.CompactionRequest{Revision: rev})
```

### 压缩原理
MVCC模块的Compact接口首先会检查Compact请求的版本号rev是否已被压缩过，若是则返回ErrCompacted错误给client。其次会检查rev是否大于当前etcd server的最大版本号，若是则返回ErrFutureRev给client

通过检查后，Compact接口会通过boltdb的API在meta bucket中更新当前已调度的压缩版本号(scheduledCompactedRev)号，然后将压缩任务追加到FIFO Scheduled中，异步调度执行。

![compact](./images/compact_3.png)

Compact接口需要持久化鵆当前已调度的压缩版本号到boltdb中，因为如果不保存，etcd在异步执行Compact任务过程中crash了，那么异常节点重启后，各个节点数据就会不一致。因此，etcd通过持久化存储scheduledCompactedRev，节点Crash重启后，会重新向FIFO Scheduled中添加压缩任务，保证各个节点间的数据一致性

treeIndex索引模块，是etcd支持保存历史版本的核心模块，每个key在treeIndex模块中都有一个keyIndex数据结构，记录其历史版本号信息

![compact](./images/compact_4.png)

异步压缩任务的第一项工作，就是**压缩treeIndex模块中的各key的历史版本**、已删除的版本。为了避免压缩工作影响读写性能，首先会克隆一个B-tree，然后通过克隆后的B-tree遍历每一个keyIndex对象，压缩历史版本号、清理已删除的版本

假设当前压缩的版本号是CompactedRev，它会保留keyIndex中最大的版本号，移除小于等于CompactedRev的版本号，并通过一个map记录treeIndex中有效的版本号返回给boltdb模块使用。这里因为最大版本号是这个key的最新版本，移除了会导致key丢失。而Compact的目的是回收旧版本。如果keyIndex中的最大版本号被打了删除标记（tombstone），就会从treeIndex中删除这个keyIndex，否则会出现内存泄露

Compact任务执行完索引压缩后，它通过遍历B-tree、keyIndex中的所有generation获得当前内存索引模块中有效的版本号，这些信息将帮助etcd清理boltdb中的废弃历史版本

![compact](./images/compact_5.png)

压缩任务的第二项工作，就是**清理boltdb中的废弃历史版本**。scheduleCompaction任务会根据key区间，从0到CompactedRev遍历boltdb中的所有key，通过treeIndex模块返回的有效索引信息，判断这个key是否有效，无效则调用boltdb的delete接口将key-value删除

在这过程中，scheduleCompaciton任务还会更新当前etcd已经完成的压缩版本号（finishedCompactRev），将其保存到boltdb的meta bucket中

scheduleCompaction任务遍历、删除key的过程可能会对boltdb造成压力，为了不影响读写请求，在执行过程中会通过参数控制每次遍历、删除的key数（默认为100，每批间隔10ms），分批完成boltdb key的删除操作

### 为什么压缩后db大小不减少？
boltdb将db文件划分成若干个page页，page页又有四种类型，分别是meta page、branch page、leaf page以及freelist page。branch page保存B+ tree的非叶子节点key数据，leaf page保存bucket和key-value数据，freelist会记录哪些页是空闲的

当我们通过boltdb删除大量的key，在事务提交后B+ tree经过分裂、平衡，会释放出若干branch/leaf page页面，然而boltdb并不会将其释放给磁盘，调整db大小操作是昂贵的，会对性能有较大的损害

boltdb是通过freelist page记录这些空闲页的分布位置，当收到新的写请求时，优先从空闲页数组中申请若干连续页使用，实现高性能的读写（而不是直接扩大db大小）。当连续空闲页申请无法得到满足的时候，boltdb才会通过增大db大小来补充空闲页

一般情况下，压缩操作释放的空闲页就能满足后续新增写请求的空闲页需求，db大小会趋于整体稳定

## 为啥基于Raft实现的etcd还会出现数据不一致？
现象: 用户在更新Kubernetes集群中的Deployment资源镜像后，无法创建出新Pod，Deployment控制器莫名其妙不工作了。更诡异的是，部分Node莫名奇妙消失了（其实Node在）

### 消失的Node
排查思路: APIServer -> ControllerManager -> Scheduler 等组件，之后查看 etcd 集群各节点状态，发现都没有问题

结果: 基于Raft实现的强一致性存储竟然出现不一致、数据丢失（其实并非Raft本身设计缺陷，而是实现层面的bug、边缘场景或运维问题）

备注: 上面的issue在旧版本（尤其是v3.5之前）确实存在此问题。但是最新版本已基本解决

根据etcd写流程，来排查问题
![raft](./images/raft_1.png)

猜测:

1. etcd集群出现分裂，三个节点分裂成两个集群。APIServer配置的后端etcd server地址是三个节点，APIServer并不会检查各节点集群ID是否一致，因此如果分裂，有可能会出现数据「消失」现象

2. Raft日志同步异常，其他两个节点会不会因为Raft模块存在特殊Bug导致未收到相关日志条目？这种情况可以通过etcd自带的WAL工具来判断，其可以显示WAL日志中收到的命令（4，5，6）

3. 如果日志同步没有问题，那么是不是Apply模块出现了问题，导致日志条目未被应用到MVCC模块？（7）

4. 如果Apply模块执行了相关日志条目到MVCC模块，MVCC模块的treeIndex子模块会不会出现了特殊Bug？导致更新流程失败？（8）

5. 如果MVCC模块的treeIndex模块无异常，写请求到了boltdb存储模块，有没有可能boltdb出现了极端异常导致丢失数据呢？（9）

开始不断抽丝剥茧、明察秋毫、一步一步探寻真相

首先从故障定位第一工具「日志」开始。查看etcd节点日志没发现任何异常日志，但是当查看APIServer日志的时候，发现持续出现「required revision has been compacted」错误，原因一般是「APIServer请求etcd版本号被压缩」

通过如下命令查看etcd节点的详细状态信息

```sh
etcdctl endpoint status --cluster -w json | jq
```

1. 判断集群是否分裂，集群中的所有节点 cluster id 是否一致

2. 初步判断集群Raft日志条目同步正常，raftIndex表示Raft日志索引号，raftAppliedIndex表示当前状态机应用的日志索引号（这里我本机的因为没问题，所以每个节点的raftIndex和raftAppliedIndex相同，但是三个节点的raftIndex和raftAppliedIndex不同）这两个核心字段显示三个节点相差很小，考虑到正在写入，未偏离正常范围，Raft同步Bug导致数据丢失也大概率可以排除，最好还是用WAL工具验证下现在日志条目同步和写入WAL是否正常

验证Raft日志同步正常

首先写入一个值，然后马上在各个节点上用WAL工具etcd-dump-logs搜索
![raft](./images/raft_2.png)

```sh
etcdctl put hello_ world_

etcd-dump-logs ./infra1.etcd/ | grep hello_

etcd-dump-logs ./infra2.etcd/ | grep hello_

etcd-dump-logs ./infra3.etcd/ | grep hello_
```

如果都能找到，那么说明日志条目同步正常

3. 观察三个节点的revision值（我本机都相同），相互之间最大差距如果过大，有明显偏离标准值

源码面前了无秘密🤫，etcd更新raftAppliedIndex核心代码如下所示。Apply流程出现逻辑错误时，并没有重试机制。etcd无论Apply流程是成功还是失败，都会更新raftAppliedIndex值。也就是说一个请求在Apply或MVCC模块即便执行失败了，都依然会更新raftAppliedIndex

```go
// ApplyEntryNormal apples an EntryNormal type Raftpb request to the EtcdServer
func （s *EtcdServer） ApplyEntryNormal（e *Raftpb.Entry） {
   shouldApplyV3 := false
   if e.Index > s.consistIndex.ConsistentIndex（） {
      // set the consistent index of current executing entry
      s.consistIndex.setConsistentIndex（e.Index）
      shouldApplyV3 = true
   }
   defer s.setAppliedIndex（e.Index）
   ....
}
```

最新版本的代码如下所示

```go
// applyEntryNormal applies an EntryNormal type raftpb request to the EtcdServer
func (s *EtcdServer) applyEntryNormal(e *raftpb.Entry, shouldApplyV3 membership.ShouldApplyV3) {
	if shouldApplyV3 {
		defer func() {
			// The txPostLockInsideApplyHook will not get called in some cases,
			// in which we should move the consistent index forward directly.
			newIndex := s.consistIndex.ConsistentIndex()
			if newIndex < e.Index {
				s.consistIndex.SetConsistentIndex(e.Index, e.Term)
			}
		}()
	}

	// raft state machine may generate noop entry when leader confirmation.
	// skip it in advance to avoid some potential bug in the future
	if len(e.Data) == 0 {
		s.firstCommitInTerm.Notify()

		// promote lessor when the local member is leader and finished
		// applying all entries from the last term.
		if s.isLeader() {
			s.lessor.Promote(s.Cfg.ElectionTimeout())
		}
		return
	}

	ar, id := apply.Apply(s.lg, e, s.uberApply, s.w, shouldApplyV3)

	// do not re-toApply applied entries.
	if !shouldApplyV3 {
		return
	}

	if ar == nil {
		return
	}

	if !errorspkg.Is(ar.Err, errors.ErrNoSpace) || len(s.alarmStore.Get(pb.AlarmType_NOSPACE)) > 0 {
		s.w.Trigger(id, ar)
		return
	}

	lg := s.Logger()
	lg.Warn(
		"message exceeded backend quota; raising alarm",
		zap.Int64("quota-size-bytes", s.Cfg.QuotaBackendBytes),
		zap.String("quota-size", humanize.Bytes(uint64(s.Cfg.QuotaBackendBytes))),
		zap.Error(ar.Err),
	)

	s.GoAttach(func() {
		a := &pb.AlarmRequest{
			MemberID: uint64(s.MemberID()),
			Action:   pb.AlarmRequest_ACTIVATE,
			Alarm:    pb.AlarmType_NOSPACE,
		}
		s.raftRequest(s.ctx, pb.InternalRaftRequest{Alarm: a})
		s.w.Trigger(id, ar)
	})
}
```

对比一下可以发现
   
   1. 重构 shouldApplyV3 的决定逻辑，更精细的控制，避免内部硬编码判断，修复了旧版本中的逻辑缺陷
   2. Consistent Index前进更加安全
   3. 更好的边缘case处理
   4. Quota报警处理更健壮

三个节点revision差异偏离标准值，恰好又说明异常etcd节点可能未成功应用日志条目到MVCC模块。所以可以通过查看MVCC的相关metrics来判断（etcd_mvcc_put_totol），来排除请求是否到了MVCC模块，事实是丢数据节点的metrics指标值的确远远落后于正常节点

所以真凶在Apply流程上，所以在Apply流程未向MVCC模块提交请求前以及可能提前返回的地方，都加上日志，然后再走一遍流程，马上出现一条错误日志「auth: revision in header is old」

写入成功还跟client连接的节点有关，连接不同节点会出现不同的写入结果

数据不一致是因为鉴权版本号不一致导致的，节点在Apply流程的时候，会判断Raft日志条目中的请求鉴权版本号是否小于当前鉴权版本号，如果小于就拒绝写入

那就得去看可能修改鉴权版本号的源码分析了🧐，只有鉴权相关的接口才会修改它，要解决就再次复现😅

要基于混沌工程，不断模拟真实业务场景、访问鉴权接口、注入故障（停止etcd进程等）

真相大白: 当无意间重启etcd的时候，如果最后一条命令是鉴权相关的，它并不会持久化consistent index（KV接口会持久化）consistent key具有幂等作用，可防止命令重复执行，consistent index的未持久化最终导致鉴权命令重复执行。恰好鉴权模块的RoleGrantPermission接口未实现幂等，重复执行会修改鉴权版本号。一连串的Bug最终导致鉴权号出现不一致，最后又放大成MVCC模块的key-value数据不一致，导致严重的数据损坏。etcd v3.3.21和v3.4.8后的版本已经修复此Bug。

### 为什么会不一致
etcd各个节点数据一致性是基于raft算法的日志复制实现的，etcd是个基于复制状态机实现的分布式系统。下图是分布式复制状态机原理架构，核心由三个组件组成，一致性模块、日志、状态机，其工作流程如下：

![raft](./images/raft_4.png)

1. client发起一个写请求（put x = 3）

2. server向一致性模块（假设是Raft）提交请求，一致性模块生成一个写提案日志条目。如果server是Leader，把日志条目广播给其他节点，并持久化日志条目到WAL中

3. 当一半以上的节点持久化日志条目后，Leader的一致性模块将此日志条目标记为已提交（committed），并通知其他节点提交

4. server从一致性模块获取已经提交的日志条目，异步应用到状态机持久化存储中（boltdb等），然后返回给client

在基于复制状态机实现的分布式存储系统中，Raft等一致性算法只能确保各个节点的日志一致性，也就是流程2

对于流程3来说，server从日志里面获取已经提交的日志条目，将其应用到状态机的过程，跟Raft算法本身无关，属于server本身的数据存储逻辑

也就是说可能存在「server应用日志到状态机失败，进而导致各个节点出现数据不一致」的情况，但是这个不一致并非Raft模块导致的，它已超过Raft模块的功能界限

Node莫名奇妙消失，就是应用日志条目到状态机流程中，出现逻辑错误，导致key-value数据未能持久化存储到boltdb中

### 其他典型不一致Bug

这个故障对外的表现也是令人摸不着头脑，有服务不调度的、有service下的endpoint不更新的。最终我经过一番排查发现，原来数据不一致是由于etcd 3.2和3.3版本Lease模块的Revoke Lease行为不一致造成

etcd 3.2版本的RevokeLease接口不需要鉴权，而etcd 3.3 RevokeLease接口增加了鉴权，因此当你升级etcd集群的时候，如果etcd 3.3版本收到了来自3.2版本的RevokeLease接口，就会导致因为没权限出现Apply失败，进而导致数据不一致，引发各种诡异现象

除了重启etcd、升级etcd可能会导致数据不一致，defrag操作也可能会导致不一致

对一个defrag碎片整理来说，它是如何触发数据不一致的呢？ 触发的条件是defrag未正常结束时会生成db.tmp临时文件。这个文件可能包含部分上一次defrag写入的部分key/value数据，而etcd下次defrag时并不会清理它，复用后就可能会出现各种异常场景，如重启后key增多、删除的用户数据key再次出现、删除user/role再次出现等

etcd 3.2.29、etcd 3.3.19、etcd 3.4.4后的版本都已经修复这个Bug。建议根据自己实际情况进行升级，否则踩坑后，数据不一致的修复工作是非常棘手的，风险度极高。

「**算法一致性不代表一个庞大的分布式系统工程实现中一定能保障一致性，工程实现上充满着各种挑战，从不可靠的网络环境到时钟、再到人为错误、各模块间的复杂交互等，几乎没有一个存储系统能保证任意分支逻辑能被测试用例100%覆盖。**」🤯

复制状态机在给我们带来数据同步的便利基础上，也给上层逻辑开发提出了高要求。也就是说任何接口逻辑变更etcd需要保证兼容性，否则就很容易出现Apply流程失败，导致数据不一致

### 最佳实践

#### 开启etcd的数据毁坏检测功能

etcd不仅支持在启动的时候，通过–experimental-initial-corrupt-check参数检查各个节点数据是否一致，也支持在运行过程通过指定–experimental-corrupt-check-time参数每隔一定时间检查数据一致性

其实无非就是想确定boltdb文件里面的内容跟其他节点内容是否一致。因此我们可以枚举所有key value，然后比较即可

etcd的实现也就是通过遍历treeIndex模块中的所有key获取到版本号，然后再根据版本号从boltdb里面获取key的value，使用crc32 hash算法，将bucket name、key、value组合起来计算它的hash值

如果开启了–experimental-initial-corrupt-check，启动的时候每个节点都会去获取peer节点的boltdb hash值，然后相互对比，如果不相等就会无法启动

而定时检测是指Leader节点获取它当前最新的版本号，并通过Raft模块的ReadIndex机制确认Leader身份。当确认完成后，获取各个节点的revision和boltdb hash值，若出现Follower节点的revision大于Leader等异常情况时，就可以认为不一致，发送corrupt告警，触发集群corruption保护，拒绝读写

### 应用层的数据一致性检测

数据不一致在MVCC、boltdb会出现很多种情况，比如说key数量不一致、etcd逻辑时钟版本号不一致、MVCC模块收到的put操作metrics指标值不一致等等。因此我们的应用层检测方法就是基于它们的差异进行巡检

首先针对key数量不一致的情况，我们可以实现巡检功能，定时去统计各个节点的key数，这样可以快速地发现数据不一致，从而及时介入，控制数据不一致影响，降低风险

统计节点key数时，记得查询的时候带上WithCountOnly参数。etcd从treeIndex模块获取到key数后就及时返回了，无需访问boltdb模块。如果数据量非常大（涉及到百万级别），那即便是从treeIndex模块返回也会有一定的内存开销，因为它会把key追加到一个数组里面返回

而在WithCountOnly场景中，我们只需要统计key数即可。对百万级别的key来说，WithCountOnly时内存开销从数G到几乎零开销，性能也提升数十倍

其次我们可以基于endpoint各个节点的revision信息做一致性监控。一般情况下，各个节点的差异是极小的

我们还可以基于etcd MVCC的metrics指标来监控。比如上面提到的mvcc_put_total，理论上每个节点这些MVCC指标是一致的，不会出现偏离太多

### 定时数据备份

etcd数据不一致的修复工作极其棘手。发生数据不一致后，各个节点可能都包含部分最新数据和脏数据。如果最终无法修复，那就只能使用备份数据来恢复了

因此备份特别重要，备份可以保障我们在极端场景下，能有保底的机制去恢复业务。请记住，在做任何重要变更前一定先备份数据，以及在生产环境中建议增加定期的数据备份机制（比如每隔30分钟备份一次数据）

可以使用开源的etcd-operator中的backup-operator去实现定时数据备份，它可以将etcd快照保存在各个公有云的对象存储服务里面

### 良好的运维规范

首先是确保集群中各节点etcd版本一致。若各个节点的版本不一致，因各版本逻辑存在差异性，这就会增大触发不一致Bug的概率

其次是优先使用较新稳定版本的etcd。像上面提到的3个不一致Bug，在最新的etcd版本中都得到了修复。可以根据自己情况进行升级，以避免下次踩坑。同时可根据实际业务场景以及安全风险，来评估是否有必要开启鉴权，开启鉴权后涉及的逻辑更复杂，有可能增大触发数据不一致Bug的概率

最后是在升级etcd版本的时候，需要多查看change log，评估是否存在可能有不兼容的特性。在你升级集群的时候注意先在测试环境多验证，生产环境务必先灰度、再全量。