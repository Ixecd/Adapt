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