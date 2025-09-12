package main

import (
	"fmt"
	"sync"
)

type HostInfo struct {
	ip       uint32
	port     uint32
	overload bool
}

type LoadBalance struct {
	modid        uint32
	cmdid        uint32
	idelList     []*HostInfo
	overloadList []*HostInfo
	probe        uint32
}



type DNSService struct {
	mutex   sync.Mutex
	lb      map[uint64]*LoadBalance
	hostMap map[uint64]*HostInfo
}

func (l *LoadBalance) GetHostInfo() *HostInfo {
	if l.probe > 10 {
		l.probe = 0
		if len(l.overloadList) > 0 {
			host := l.overloadList[0]
			l.overloadList = l.overloadList[1:]
			return host
		}
	}
	if len(l.idelList) > 0 {
		host := l.idelList[0]
		l.idelList = l.idelList[1:]
		l.probe++
		return host
	}
	return nil
}

func (d *DNSService) GetHostInfo(modid, cmdid uint32) *HostInfo {
	return d.lb[uint64(modid)<<32+uint64(cmdid)].GetHostInfo()
}

func (d *DNSService) UpdateHostInfo(modid, cmdid uint32, host *HostInfo) {
	d.mutex.Lock()
	defer d.mutex.Unlock()
	d.hostMap[uint64(host.ip)<<32+uint64(host.port)] = host
	d.lb[uint64(modid)<<32+uint64(cmdid)].UpdateList(host)
}

func (lb *LoadBalance) UpdateList(host *HostInfo) {
	if host.overload {
		lb.overloadList = append(lb.overloadList, host)
	} else {
		lb.idelList = append(lb.idelList, host)
	}
}

func (lb *LoadBalance) AdjustList(host *HostInfo) {

}

func (d *DNSService) DeleteHostInfo(host *HostInfo) {
	d.mutex.Lock()
	defer d.mutex.Unlock()
	delete(d.hostMap, uint64(host.ip)<<32+uint64(host.port))
}

func main_test() {
	dns := &DNSService{
		mutex:   sync.Mutex{},
		lb:      make(map[uint64]*LoadBalance),
		hostMap: make(map[uint64]*HostInfo),
	}

	modid, cmdid := 1, 1

	modid2, cmdid2 := 2, 2

	dns.lb[uint64(modid)<<32+uint64(cmdid)] = &LoadBalance{
		modid:        uint32(modid),
		cmdid:        uint32(cmdid),
		idelList:     make([]*HostInfo, 0),
		overloadList: make([]*HostInfo, 0),
		probe:        10,
	}

	dns.lb[uint64(modid2)<<32+uint64(cmdid2)] = &LoadBalance{
		modid:        uint32(modid2),
		cmdid:        uint32(cmdid2),
		idelList:     make([]*HostInfo, 0),
		overloadList: make([]*HostInfo, 0),
		probe:        20,
	}

	dns.lb[uint64(modid)<<32+uint64(cmdid2)] = &LoadBalance{
		modid:        uint32(modid),
		cmdid:        uint32(cmdid2),
		idelList:     make([]*HostInfo, 0),
		overloadList: make([]*HostInfo, 0),
		probe:        10,
	}

	dns.lb[uint64(modid2)<<32+uint64(cmdid2)].UpdateList(&HostInfo{
		ip:       500,
		port:     500,
		overload: false,
	})

	dns.lb[uint64(modid)<<32+uint64(cmdid)].UpdateList(&HostInfo{
		ip:       100,
		port:     100,
		overload: false,
	})

	dns.lb[uint64(modid2)<<32+uint64(cmdid2)].UpdateList(&HostInfo{
		ip:       200,
		port:     200,
		overload: false,
	})

	host := dns.GetHostInfo(uint32(modid), uint32(cmdid))
	if host == nil {
		fmt.Println("host is nil")
	}

	fmt.Println("host ip = ", host.ip)
	fmt.Println("host port = ", host.port)

	host2 := dns.GetHostInfo(uint32(modid2), uint32(cmdid2))
	if host2 == nil {
		fmt.Println("host2 is nil")
	}

	fmt.Println("host2 ip = ", host2.ip)
	fmt.Println("host2 port = ", host2.port)
}

func traverseStringToInt(str string) int {
	temp, res := 0, 0
	for _, v := range str {
		if (v == '.') {
			res = res*256 + temp
			temp = 0
		}
		temp = temp*10 + int(v-'0')
	}
	res = res*256 + temp
	return res
}

func main() {
	fmt.Println(traverseStringToInt("192.168.1.1"))
	fmt.Println(traverseStringToInt("19.21.68.11"))
	fmt.Println(traverseStringToInt("192.168.1.1"))
}
