```fish
# 获取当前的资源 pod
$ kubectl get pod
	-A, --all-namespaces 查看当前所有名称空间的资源
	-n  指定名称空间, 默认default, kube-system 空间存放的是当前组件资源
	--show-labels  查看当前的标签
	-l  筛选资源, label缩写, key, key=value
	-o wide  pod的详细信息包括每个容器的ip和分配Node信息
	
# 进入Pod内部的容器执行命令
$ kubectl exec -it podName -c cName -- command
	-c 如果Pod内部只有一个容器,可以省略-c

# 查看资源的描述
$ kubectl explain pod.sepc

# 查看pod内部容器的日志
$ kubectl logs podName -c cName
	如果Pod内部只有一个容器,-c可省略

# 查看资源对象的详细描述 
$ kubectl describe pod podName
```

## 往github传镜像的时候,镜像太多缓存不够不能push成功,分批次push