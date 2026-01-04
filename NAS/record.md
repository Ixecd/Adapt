## NAS 内网穿透与旁路由组网全纪录

### 一、核心痛点：为什么要做内网穿透？
在没有公网 IP 的环境下，想要在外网随时访问家里的 NAS 和路由器，传统方案（如 QuickConnect）速度慢且受限。我们采用 ZeroTier (虚拟局域网) + OpenWrt (旁路由) 的方案，实现 P2P 直连，速度起飞。😤

### 二、关键概念扫盲
1. **SD-WAN（软件定义广域网）**:这是ZeroTier的本质。它通过软件在公网上虚拟出一根网线，把异地的设备串联在同一个二层网络。
2. **NAT穿透与P2P**:通过UDP打洞技术。如果显示`DIRECT`，说明数据不走服务器中转，而是两台设备点对点直连。
3. **旁路由回程路由问题**:关键难点。NAS的网关如果没有指向OpenWrt，它收到外网请求后会把相应发送给主路由，导致连接超时。
4. **MTU & MSS锁定**: VPN隧道会导致数据包增大，超过物理网口承载量。通过 `TCPMSS` 强制切片，解决网页持续 Loading转圈、资源加载不全的问题。

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
- 解决:
```bash
iptables -t nat -A POSTROUTING -d 192.168.1.0/24 -j MASQUERADE
```
5.解决网页Loading卡段: MSS调优
- 现象: 手机通过5G信号访问5000端口时，一直loading转圈
- 解决:
```bash
iptables -t mangle -I FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1200
```