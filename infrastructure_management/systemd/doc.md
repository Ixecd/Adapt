## Systemd 入门

首先 Systemd 是Linux系统工具，用来启动**守护进程**，Mac上通过launchd来替代。

Systemd 为系统的启动和管理提供一套完整的解决方案。

Systemd 取代了 initd，成为了系统的第一个进程(PID = 1，孤儿进程也由其管理)，其他进程都是它的子进程，但是注意：在容器环境、用户级systemd、调试和测试环境中，systemd的pid不一定为1。

### 系统管理
Systemd并不是一个命令，而是一组命令，涉及到系统管理的各个方面
**systemctl**
systemctl是Systemd的主命令，用于管理系统。
```bash
# 重启系统
$ sudo systemctl reboot

# 关闭系统，切断电源
$ sudo systemctl poweroff

# CPU停止工作
$ sudo systemctl halt

# 暂停系统
$ sudo systemctl suspend

# 让系统进入冬眠状态
$ sudo systemctl hibernate

# 让系统进入交互式休眠状态
$ sudo systemctl hybrid-sleep

# 启动进入救援状态(单用户状态)
$ sudo systemctl rescure
```
**systemd-analyze**命令用户查看启动耗时
```bash
# 查看启动耗时
$ systemd-analyze

# 查看每个服务的启动耗时
$ systemd-analyze blame

# 显示瀑布状的启动过程流程
$ systemd-analyze critical-chain

# 显示指定服务的启动流
$ systemd-analyze critical-chain svc.service
```
**hostnamectl**用于查看当前主机的信息
```bash
# 显示当前主机的信息
$ hostnamectl

# 设置主机名
$ sudo hostnamectl set-hostname qc
```
**localectl**命令用于查看本地化设置
```bash
# 查看本地化设置
$ localectl

# 设置本地化参数
$ sudo localectl set-local LANG=en_GB.utf8
$ sudo localectl set-keymap en_GB
```
**timedatectl**查看当前时区设置
```bash
# 查看当前时区设置
$ timedatectl

# 显示所有可用的时区
$ timedatectl list-timezones

# 设置当前时区
$ sudo timedatectl set-timezone China/Shanghai
$ sudo timedatectl set-time YYYY-MM-DD
$ sudo timedatectl set-time HH::MM::SS
```
**loginctl** 用于查看当前登录的用户
```bash
# 列出当前的session
$ loginctl list-sessions

# 列出当前登录用户
$ loginctl list-users

# 列出指定用户信息
$ loginctl show-user qc
```
**Unit**
Systemd可以管理所有系统资源。不同的资源统称为Unit(单位)，一共12种
- Service unit: 系统服务
- Target unit: 多个Unit构成的一个组
- Device unit: 硬件设备
- Mount unit: 文件系统的挂载点
- Automount unit: 自动挂载点
- Path unit: 文件或路径
- Scope unit: 不是由 Systemd 启动的外部进程
- Slice unit: 进程组
- Snapshot unit: Systemd 快照，可以切回某个快照
- Socket unit: 进程间通信的socket
- Swap unit: swap文件
- Timer unit: 定时器
```bash
# 列出正在运行的Unit
$ systemctl list-units

# 列出所有Unit，包括没有找到配置文件的或者启动失败的
$ systemctl list-units --all

# 列出所有没有运行的Unit
$ systemctl list-units --all --state=inactive

# 列出所有加载失败的Unit
$ systemctl list-units --failed

# 列出所有正在运功的、类型为service的Unit
$ systemctl list-units --type=serivce
```
**Unit的状态**
systemctl status 命令用户查看系统状态和单个Unit的状态
```bash
# 显示系统状态
$ systemctl status

# 显示单个 Unit 的状态
$ systemctl status bluetooth.service

# 显示远程主机的某个Unit状态
$ systemctl -H root@qc7.top status httpd.service

# 显示某个Unit是否正在运行
$ systemctl is-active application.service

# 显示某个Unit是否处于启动失败状态
$ systemctl is-failed application.service

# 显示某个Unit服务是否建立了启动链接
$ systemctl is-enabled application.service
```
**Unit管理**
```bash
# 立即启动一个服务
$ sudo systemctl start apache.service

# 立即停止一个服务
$ sudo systemctl stop apache.service

# 重新启动一个服务
$ sudo systemctl restart apache.service

# 杀死一个服务的所有子进程
$ sudo systemctl kill apache.service

# 重新加载一个服务的配置文件
$ sudo systemctl reload apache.service

# 重载所有修改过的配置文件
$ sudo systemctl daemon-reload

# 显示某个unit的所有底层参数
$ systemctl show httpd.service

# 显示某个unit的指定属性的值
$ systemctl show -p CPUShares httpd.service

# 设置某个unit的指定属性
$ sudo systemctl set-proerty httpd.service CPUShares=500
```
**依赖关系**
Unit之间存在依赖关系，如果A服务依赖于B服务，那么当systemd在启动A服务时，会同时启动B服务
```bash
# 列出一个Unit的所有依赖
$ systemctl list-dependencies nginx.service

# 列出所有依赖类型包括Target类型
$ systemctl list-dependencies --all nginx.service
```

**Unit配置文件**
每个unit都有一个配置文件，告诉systemd如何来启动这个Unit
Systemd默认从目录`/etc/systemd/system`读取配置文件。但是，里面存放的大部分文件都是符号链接，执行目录`/usr/lib/systemd/system/`，真正的配置文件存放在那个目录
而`systemctl enable`用于在上面两个目录之间，建立符号链接
```bash
$ sudo systemctl enable clamd@scan.service
# 等同于
$ sudo ln -s '/usr/lib/systemd/system/clamd@scan.service' '/etc/systemd/system/multi-user.target.wants/clamd@scan.service'
```
如果配置文件里面设置了开机启动，`systemctl enable`命令相当于激活开机启动。
```bash
# 撤销符号链接，撤销开机启动
$ sudo systemctl disable xxx.service
```
配置文件的后缀名，是该Unit的种类，比如`sshd.socket`。如果省略，systemd默认后缀名为`.service`，所以sshd会被理解成sshd.service。
**配置文件的状态**
`systemctl list-unit-files`用于列出所有配置文件
```bash
# 列出所有配置文件
$ systemctl list-unit-files

# 列出指定类型的配置文件
$ systemctl list-unit-files --type=service
```
配置文件的状态
- enabled: 已建立启动链接
- disabled: 没建立启动链接
- status: 该配置文件没有install部分，也就是无法执行，只能作为其他配置文件的依赖
- masked: 该配置文件被禁止建立启动链接
一旦修改配置文件，就要让Systemd重新加载配置文件，重启启动，让配置生效
```bash
# 重新加载systemd的单元配置文件，不会重启任何服务，只是让systemd重新读取配置
$ sudo systemctl daemon-reload
# 重启httpd服务
$ sudo systemctl restart httpd.service
```
**配置文件格式**
通过`systemctl cat`来查看配置文件的内容，ini格式
**配置文件的区块**
`[Unit]`区块通常是配置文件的第一个区块，用来定义Unit的元数据，以及配置与其他Unit的关系。
- Description：简短描述
- Documentation：文档地址
- Requires：当前 Unit 依赖的其他 Unit，如果它们没有运行，当前 Unit 会启动失败
- Wants：与当前 Unit 配合的其他 Unit，如果它们没有运行，当前 Unit 不会启动失败
- BindsTo：与Requires类似，它指定的 Unit 如果退出，会导致当前 Unit 停止运行
- Before：如果该字段指定的 Unit 也要启动，那么必须在当前 Unit 之后启动
- After：如果该字段指定的 Unit 也要启动，那么必须在当前 Unit 之前启动
- Conflicts：这里指定的 Unit 不能与当前 Unit 同时运行
- Condition...：当前 Unit 运行必须满足的条件，否则不会运行
- Assert...：当前 Unit 运行必须满足的条件，否则会报启动失败
`[Install]`通常是配置文件的最后一个区块，用来定义如何启动，以及是否开机启动
- WantedBy：它的值是一个或多个 Target，当前 Unit 激活时（enable）符号链接会放入/etc/systemd/system目录下面以 Target 名 + .wants后缀构成的子目录中
- RequiredBy：它的值是一个或多个 Target，当前 Unit 激活时，符号链接会放入/etc/systemd/system目录下面以 Target 名 + .required后缀构成的子目录中
- Alias：当前 Unit 可用于启动的别名
- Also：当前 Unit 激活（enable）时，会被同时激活的其他 Unit
**Target**
启动计算机的时候，需要启动大量的Unit。如果每一次启动，都要写明本次启动需要哪些Unit，非常不方便。Systemd的解决方案是Target
Target就是一个Unit组，包含许多相关的Unit。启动某个Target时，Systemd就会启动里面所有的Unit。Target这个概念类似于「状态点」，启动某个Target就好比启动到某种状态
传统的「init」启动模式里面，有RunLevel的概念，跟Target的作用类似。RunLevel是互斥的，不可能有多个同时启动，但是Target可以同时启动多个
```bash
# 查看当前系统的所有Target
$ systemctl list-unit-files --type=target

# 查看一个Target包含的所有Unit
$ systemctl list-dependencies multi-user.target

# 查看启动时的默认Target
$ systemctl get-default

# 设置启动时的默认Target
$ sudo systemctl set-default multi-user.target

# 切换Target时，默认不关闭前一个Target启动的进程
# systemctl isolate 命令改变这种行为
# 关闭前一个 Target 里面所有不属于后面一个 Target的进程
$ sudo systemctl isolate multi-user.target
```
**日志管理**
Systemd 统一管理所有Unit的启动日志，可以只用`journalctl`一个命令，查看所有日志。日志的配置文件是`/etc/systemd/journald.conf`
```bash
# 查看所有日志，默认只保存本次启动的日志
$ sudo journalctl

# 查看内核日志(不显示应用日志)
$ sudo journalctl -k

# 查看系统本次启动的日志
$ sudo journalctl -b
$ sudo journalctl -b -0

# 查看上一次启动的日志
$ sudo journalctl -b -1

# 查看指定时间的日志
$ sudo journalctl --since="2021-09-10 12:12:12"
$ sudo journalctl --since "20 min ago"
$ sudo journalctl --since yesterday
$ sudo journalctl --since "2015-01-10" --unit "2015-01-11 03:00"
$ sudo journalctl --since 09:00 --unit "1 hour ago"

# 显示尾部的最新10行日志，默认就是10
$ sudo journalctl -n

# 显示尾部的最新20行日志
$ sudo journalctl -n20

# 实时滚动显示最新日志
$ sudo journalctl -f

# 查看指定服务的日志
$ sudo journalctl /usr/lib/systemd/system/httpd.service

# 查看指定服务的日志
$ sudo journalctl _PID=1

# 查看某个路径的脚本的日志
$ sudo journalctl /usr/bin/bash

# 查看指定用户的日志
$ sudo journalctl _UID=33 --since today

# 查看某个Unit的日志
$ sudo journalctl -u nginx.service
$ sudo journalctl -u nginx.service --since today

# 实时滚动显示某个Unit的最新日志
$ sudo journalctl -u nginx.service -f

# 查看指定优先级（及其以上级别）的日志，一共8级
# 0: emerge
# 1: alert
# 2: crit
# 3: err
# 4: warning
# 5: notice
# 6: info
# 7: debug
$ sudo journalctl -p err -b

# 给JSON格式（单行）输出
$ sudo journalctl -b -u nginx.service -o json-pertty

# 显示日志占据的硬盘空间
$ sudo journalctl --disk-usage

# 指定日志文件占据的最大空间
$ sudo journalctl --vacuum-size=1G

# 指定日志文件保存时间
$ sudo journalctl --vacuum-time=1years
```