package main

import (
	"context"
	"errors"
	"fmt"
	"log"
	"strings"
	"time"

	"go.etcd.io/etcd/api/v3/mvccpb"
	clientv3 "go.etcd.io/etcd/client/v3"
)

func watchTasks2(cli *clientv3.Client, taskPrefix string) {
	// 本地缓存（生产用并发安全 map）
	localTasks := make(map[string]string)

	for {
		// 步骤1: 全量 Get 当前任务快照（处理 Get 错误重试）
		ctx := context.Background()
		resp, err := cli.Get(ctx, taskPrefix, clientv3.WithPrefix())
		if err != nil {
			log.Printf("ERROR: Get tasks failed: %v, retry in 5s", err)
			time.Sleep(5 * time.Second)
			continue
		}

		// 加载快照到本地
		for _, kv := range resp.Kvs {
			localTasks[string(kv.Key)] = string(kv.Value)
			// do something with existing tasks...
		}
		log.Printf("INFO: Synced %d existing tasks at revision %d", len(localTasks), resp.Header.Revision)

		// 步骤2: 从当前最新 revision +1 开始 Watch 未来变更
		watchChan := cli.Watch(
			ctx,
			taskPrefix,
			clientv3.WithPrefix(),
			clientv3.WithRev(resp.Header.Revision+1), // 可选：明确指定，避免狭窄窗口
			clientv3.WithProgressNotify(),            // 关键：发送心跳，防止超时断开
		)

		// 步骤3: 处理 Watch 流
		for wresp := range watchChan {
			if wresp.Err() != nil {
				// 关键错误处理
				errMsg := wresp.Err().Error()
				if strings.Contains(errMsg, "compacted") || errors.Is(wresp.Err(), context.DeadlineExceeded) {
					log.Printf("WARN: Watch compacted, resync from latest")
					break // 跳出内层，重新 Get + Watch
				}
				if wresp.IsProgressNotify() {
					log.Printf("DEBUG: Watch progress notify")
					continue
				}
				log.Printf("ERROR: Watch error: %v, resync", wresp.Err())
				break // 其他错误也重新同步
			}

			// 正常事件处理
			for _, ev := range wresp.Events {
				key := string(ev.Kv.Key)
				switch ev.Type {
				case mvccpb.PUT:
					localTasks[key] = string(ev.Kv.Value)
					log.Printf("New/Updated task: %s = %s", key, localTasks[key])
				case mvccpb.DELETE:
					delete(localTasks, key)
					log.Printf("Deleted task: %s", key)
				}
				// do something...
			}
		}

		// Watch chan 关闭 → 睡一会重试，避免疯狂循环
		log.Printf("WARN: Watch stream closed, retry in 5s")
		time.Sleep(5 * time.Second)
	}
}

func watchTasks(cli *clientv3.Client, taskPrefix string) {
	for { // 无限循环：断连自动恢复
		// 步骤1: 全量 Get 当前最新快照（只一次，不用再 Get 第二次）
		ctx := context.Background()
		getResp, err := cli.Get(ctx, taskPrefix, clientv3.WithPrefix())
		if err != nil {
			log.Printf("ERROR: Get failed: %v, retry in 5s", err)
			time.Sleep(5 * time.Second)
			continue
		}

		// 处理当前所有现有任务（你的初始化逻辑，无需缓存）
		for _, kv := range getResp.Kvs {
			fmt.Printf("Existing task: %s => %s\n", kv.Key, kv.Value)
			// do something: 执行任务、标记已处理等
		}
		log.Printf("INFO: Initialized with %d tasks at revision %d", len(getResp.Kvs), getResp.Header.Revision)

		// 步骤2: Watch 未来变更（用 Get 的 revision +1，零窗口漏事件）
		watchChan := cli.Watch(
			ctx,
			taskPrefix,
			clientv3.WithPrefix(),
			clientv3.WithRev(getResp.Header.Revision+1), // 关键：消除窗口
			clientv3.WithProgressNotify(),               // 关键：心跳防超时断
		)

		// 步骤3: 处理事件流
		for wresp := range watchChan {
			if wresp.Err() != nil {
				errMsg := wresp.Err().Error()
				if strings.Contains(errMsg, "compacted") || errors.Is(wresp.Err(), context.DeadlineExceeded) {
					log.Printf("WARN: Compacted or timeout, will resync on next loop")
				} else {
					log.Printf("ERROR: Watch error: %v, will resync", wresp.Err())
				}
				break // 任何错误/断开 → 跳出，重新 Get + Watch
			}

			if wresp.IsProgressNotify() {
				continue // 心跳忽略
			}

			// 直接处理事件（无本地 map）
			for _, ev := range wresp.Events {
				switch ev.Type {
				case mvccpb.PUT:
					fmt.Printf("New/Updated task: %s => %s\n", ev.Kv.Key, ev.Kv.Value)
					// do something: 执行新任务
				case mvccpb.DELETE:
					fmt.Printf("Deleted task: %s\n", ev.Kv.Key)
					// do something: 清理
				}
			}
		}

		// Watch 断开 → 睡一下重试
		log.Printf("WARN: Watch stream closed, retry in 5s")
		time.Sleep(5 * time.Second)
	}
}

func main() {
	cli, err := clientv3.New(clientv3.Config{
		Endpoints: []string{"http://127.0.0.1:2379", "http://127.0.0.1:22379", "http://127.0.0.1:32379"},
	})
	if err != nil {
		log.Fatalf("Failed to connect to etcd: %v", err)
	}
	defer cli.Close()

	watchTasks(cli, "tasks/")
}
