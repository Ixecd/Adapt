## NAS 内网穿透与旁路由组网全纪录

### 一、核心痛点：为什么要做内网穿透与旁路路由子组网？
在没有公网 IP 的环境下，想要在外网随时访问家里的 NAS 和路由器，传统方案（如 QuickConnect）速度慢且受限。我们采用 ZeroTier (虚拟局域网) + OpenWrt (旁路由) 的方案，实现 P2P 直连，速度起飞。😤

为了让NAS能实现科学上网，需要使用旁路路由子组网，将NAS的流量转发到OpenWrt，然后通过OpenWrt的科学上网功能实现科学上网。💪

### 二、关键概念扫盲
1. **旁路由**: 不是真正的路由器，而是主路由侧边的一个「流量处理中心」。
	- 逻辑流向: 设备 -> 旁路由(处理: 穿透/解密) -> 主路由 -> 互联网
	- 优势: 不影响主路由的正常使用，配置灵活，适合虚拟机部署，且可以实现科学上网。
2. **SD-WAN（软件定义广域网）**:这是ZeroTier的本质。它打破了物理地址的限制，在公共互联网上，通过软件抽象出一层虚拟的「大交换机」。所有设备通过虚拟网卡接入，分配一个10.70.x.x的内网IP。
	- 逻辑流向: 手机虚拟网卡 <-> 加密隧道 <-> 旁路由虚拟网卡
	- 优势: 无需公网IP，无需配置复杂的 IPsec 或 OpenVPN，所有流量在公网传输时都是经过端到端加密的，设备之间就像连在同一根物理网线上一样，可以直接通过虚拟内网IP互访。
3. **NAT穿透与P2P**:通过UDP打洞（Hole Punching）技术，让处于两个不同防火墙（NAT）后的设备建立直接连接。
	- 逻辑流向: 手机和旁路由分别向 ZeroTier 的行星服务器（Planet）报告自己的外网IP和端口，双方通过互相探测，建立直接的UDP隧道。如果成功，状态为`DIRECT`，如果失败，状态为`RELAY`。
	- 优势: 不经过第三方服务器中转，延迟仅取决于两端的物理距离，速度上线取决于手机的5G速率和家里的宽带上行速率。
4. **旁路由回程路由问题**:旁路由架构中最经典的「单向通」故障。数据包能「进去」，但 NAS 回信时 「找不到路」。
	- 逻辑流向（故障场景）
		- 去程: 手机 -> 隧道 -> 旁路由 -> NAS（成功）
		- 回程: NAS发现请求来自 `10.70.x.x`，但是NAS的默认网关是**主路由**
		- 丢弃: 主路由不认识`10.70.x.x`，将回信直接丢弃，手机端直接Timeout
	- 解决: 旁路由在转发给NAS前，把请求方的IP伪装成自己的局域网IP。NAS以为是旁路由在找它，回信给旁路由，再由旁路由通过隧道传回手机
		- 在旁路由 OpenWrt 中添加 iptables规则。执行`iptables -t nat -A POSTROUTING -j MASQUERADE`，对所有流量生效，暴力大板砖，力大砖飞，包治百病。
5. **MTU & MSS锁定**: VPN隧道会导致数据包增大，超过物理网口标准承载量（1500字节）。
	- 逻辑流向: 手机发送1500字节数据 -> 加上 ZeroTier头部 -> 变成 1540字节，物理路由无法处理大包，导致数据包被强制拆解或直接丢弃 -> 网页持续 Loading转圈、资源加载不全的问题。
	- 解决: 在TCP握手阶段，强制让双方约定一个较小的「分段大小」（比如1200字节）。执行`iptables -t mangle -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1200`

### 三、实战步骤：那些我们踩过的坑
1. 环境搭建
- 宿主机: 512MB内存的极简OpenWrt虚拟机
- 软件: ZeroTier客户端
- 操作: 通过`zerotier-cli join [ID]` 加入网络，并通过`zerotier-cli listnetworks`获取虚拟IP

2. 第一道防线: 防火墙（iptables）
- 现象: 手机可以ping通，但是打不开wrt网页
- 解决:
```bash
# 允许 ZeroTier 虚拟网口（zt+,替换为实际网口）的所有流量
iptables -I INPUT -i zt+ -j ACCEPT
iptables -I FORWARD -i zt+ -j ACCEPT
```
3. 攻克「核心关卡」:端口转发(DNAT)
- 需求: 访问 `10.70.61.xx:5000` 自动跳转到内网NAS`192.168.1.3:5000`
- 解决:
```bash
# 将ZeroTier虚拟网口（zt+,替换为实际网口）的TCP 5000端口请求转发到内网NAS的192.168.1.3:5000
iptables -t nat -I PREROUTING -i zt+ -p tcp --dport 5000 -j DNAT --to-destination 192.168.1.3:5000
```
4. 终极必杀技: 流量伪装(SNAT/MASQUERADE)
- 现象:NAS 收到请求但不回信（回程路由丢包）。
- 原理:将来自虚拟网的请求源地址伪装成 OpenWrt 自己的内网 IP。
- 解决:只对家里的设备生效，不能科学上网了，不推荐
```bash
iptables -t nat -A POSTROUTING -d 192.168.1.0/24 -j MASQUERADE
```
5.解决网页Loading卡段: MSS调优
- 现象: 手机通过5G信号访问5000端口时，一直loading转圈
- 解决:
```bash
iptables -t mangle -I FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1200
```

### 四、延迟与性能测试
1. 延迟测试（公司网络 + ZeroTier虚拟网卡）
- 测试工具: `ping`
- 测试命令: `ping -c 10 -s 1200 10.70.61.xx`
- 测试结果:
```bash
PING 10.70.61.xx (10.70.61.xx): 56 data bytes
1208 bytes from 10.70.61.xx: icmp_seq=1 ttl=64 time=23.266 ms
1208 bytes from 10.70.61.xx: icmp_seq=2 ttl=64 time=16.036 ms
1208 bytes from 10.70.61.xx: icmp_seq=3 ttl=64 time=11.605 ms
1208 bytes from 10.70.61.xx: icmp_seq=4 ttl=64 time=14.245 ms
1208 bytes from 10.70.61.xx: icmp_seq=5 ttl=64 time=11.085 ms
1208 bytes from 10.70.61.xx: icmp_seq=6 ttl=64 time=14.526 ms
1208 bytes from 10.70.61.xx: icmp_seq=7 ttl=64 time=13.822 ms
1208 bytes from 10.70.61.xx: icmp_seq=8 ttl=64 time=14.409 ms
1208 bytes from 10.70.61.xx: icmp_seq=9 ttl=64 time=20.194 ms
```
2. !!延迟测试（公司网络 + QuickConnect）
- 测试工具: `ping`
- 测试命令: `ping -c 10 -s quickconnect.cn`
- 测试结果: 不是实际的延迟
```bash
PING (28.0.0.37): 1200 data bytes
1208 bytes from 28.0.0.37: icmp_seq=0 ttl=64 time=0.538 ms
1208 bytes from 28.0.0.37: icmp_seq=1 ttl=64 time=0.359 ms
1208 bytes from 28.0.0.37: icmp_seq=2 ttl=64 time=0.394 ms
1208 bytes from 28.0.0.37: icmp_seq=3 ttl=64 time=0.356 ms
1208 bytes from 28.0.0.37: icmp_seq=4 ttl=64 time=0.325 ms
1208 bytes from 28.0.0.37: icmp_seq=5 ttl=64 time=0.385 ms
1208 bytes from 28.0.0.37: icmp_seq=6 ttl=64 time=0.343 ms
1208 bytes from 28.0.0.37: icmp_seq=7 ttl=64 time=0.447 ms
```
**PING值欺骗: 为什么Ping只有0.2ms，但是网页加载却要2秒？**

实际再通过浏览器测试发现，QuickConnect打开网页的速度明显要比ZeroTier慢，但是延迟却比ZeroTier低。

- 原因: 电脑上安装了 Synology Drive或者相关软件。这些软件会在本地运行一个「监听器」，ping quickconnect 时，电脑DNS会劫持这个请求，指向本地的一个虚拟地址(28.0.0.37)

### 五、ZeroTier + HTTP 的安全性分析
1. ZeroTier 本身提供强加密：ZeroTier 是基于 UDP 的虚拟网络层（Layer 2/3）隧道，使用 256-bit Salsa20 加密 + Poly1305 认证（类似 WireGuard 的安全级别）。所有在 ZeroTier 网络内的流量（包括 HTTP）都是端到端加密的，只有加入同一网络的授权节点才能解密。

2. 在隧道内，HTTP 流量不会明文暴露：外部攻击者（比如公网嗅探、ISP）无法看到您的用户名、密码或文件内容，因为整个数据包都在加密隧道里传输。这比直接公网暴露 HTTP 要安全得多。

3. 实际风险很低：在家庭/个人场景下，如果您的 ZeroTier 网络只授权了可信设备（手机、电脑），且节点没有被恶意软件感染，HTTP 就已经足够安全。很多用户长期这样用，没有出过问题。

4. 推荐再加一层HTTPS，安全更加彻底，几乎无成本（买个域名，从Let's Encrypt申请免费证书）