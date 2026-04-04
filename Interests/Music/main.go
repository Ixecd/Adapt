package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

func main() {
	root := "./Divine"
	entries, _ := os.ReadDir(root)

	for _, entry := range entries {
		if entry.IsDir() {
			filePath := filepath.Join(root, entry.Name(), "Lines.md")
			if _, err := os.Stat(filePath); err == nil {
				processFile(filePath)
			}
		}
	}
	fmt.Println("--- All Systems Go: 最终效果已拿捏 ---")
}

func processFile(path string) {
	content, _ := os.ReadFile(path)
	lines := strings.Split(string(content), "\n")

	var rawLines []string
	var title string

	for _, line := range lines {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}
		// 如果是标题行，单独拎出来
		if strings.HasPrefix(line, "##") {
			title = line
			continue
		}
		// 还原数据：去掉所有已有的 Markdown 符号
		line = strings.TrimPrefix(line, ">")
		line = strings.TrimPrefix(strings.TrimSpace(line), "**")
		line = strings.TrimSuffix(strings.TrimSpace(line), "**")
		line = strings.TrimSpace(line)

		if line != "" {
			rawLines = append(rawLines, line)
		}
	}

	if len(rawLines) == 0 {
		return
	}

	var output strings.Builder
	// 如果有标题，先写标题，标题不进引用块
	if title != "" {
		output.WriteString(title + "\n\n")
	}

	for i := 0; i < len(rawLines); i += 2 {
		// 1. 英文行：原样输出 + 两个空格换行
		output.WriteString(fmt.Sprintf("> %s  \n", rawLines[i]))

		// 2. 中文行：加粗
		if i+1 < len(rawLines) {
			output.WriteString(fmt.Sprintf("> **%s** \n", rawLines[i+1]))
		}

		// 3. 组间空行
		if i+2 < len(rawLines) {
			output.WriteString("> \n")
		}
	}

	os.WriteFile(path, []byte(output.String()), 0644)
	fmt.Printf("Done: %s\n", path)
}