package main

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

func main() {
	// 获取当前工作目录
	root := "."

	// 遍历当前目录下的所有子目录
	entries, err := os.ReadDir(root)
	if err != nil {
		fmt.Printf("读取目录失败: %v\n", err)
		return
	}

	for _, entry := range entries {
		if entry.IsDir() {
			// 构建 Lines.md 的路径，例如 ./Ghost/Lines.md
			filePath := filepath.Join(root, entry.Name(), "Lines.md")

			// 检查该目录下是否存在 Lines.md
			if _, err := os.Stat(filePath); err == nil {
				processFile(filePath)
			}
		}
	}
	fmt.Println("--- All Systems Go: 所有 Lines.md 处理完毕 ---")
}

func processFile(path string) {
	fmt.Printf("正在拿捏: %s\n", path)

	file, err := os.Open(path)
	if err != nil {
		fmt.Printf("无法打开文件 %s: %v\n", path, err)
		return
	}

	var rawLines []string
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		// 简单的幂等检查：如果已经是以 ">" 或 "> **" 开头，说明可能处理过了，跳过空行
		if line == "" || strings.HasPrefix(line, ">") {
			continue
		}
		rawLines = append(rawLines, line)
	}
	file.Close()

	if len(rawLines) == 0 {
		return
	}

	// 开始重新构建格式
	var output strings.Builder
	for i := 0; i < len(rawLines); i += 2 {
		// 英文行：加粗并添加引用前缀
		output.WriteString(fmt.Sprintf("> **%s**\n", rawLines[i]))

		// 中文行
		if i+1 < len(rawLines) {
			output.WriteString(fmt.Sprintf("> %s\n", rawLines[i+1]))
		}

		// 组间空行（带引用符以保持垂直线连续）
		if i+2 < len(rawLines) {
			output.WriteString("> \n")
		}
	}

	// 将处理后的内容写回原文件
	err = os.WriteFile(path, []byte(output.String()), 0644)
	if err != nil {
		fmt.Printf("写入文件失败 %s: %v\n", path, err)
	}
}