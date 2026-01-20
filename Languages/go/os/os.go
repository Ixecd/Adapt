package main

import (
	"fmt"
	"io"
	"log"
	"os"
	"os/exec"
	"strconv"
)

func parent() {
	// 通过 Args 区分父子进程
	cmd := exec.Command(os.Args[0], "-child")
	cmd.Stdout = os.Stdout

	// 文件总数
	files := []string{"os.md"}
	cmd.Env = append(cmd.Env, fmt.Sprintf("count=%d", len(files)))

	for i := 0; i < len(files); i++ {
		name := files[i]

		file, err := os.Open(name)
		if err != nil {
			log.Fatalln(err)
		}

		defer file.Close()

		cmd.Env = append(cmd.Env, fmt.Sprintf("%d=%s", 3+i, name))
		cmd.ExtraFiles = append(cmd.ExtraFiles, file)
	}

	if err := cmd.Run(); err != nil {
		log.Fatalln(err)
	}
}

func child() {
	// 获取文件总数
	count, _ := strconv.Atoi(os.Getenv("count"))

	// 从3+i开始，读取文件名和内容
	for i := 0; i < count; i++ {
		name := os.Getenv(strconv.Itoa(3+i))

		file := os.NewFile(uintptr(3+i), name)
		defer file.Close()

		b, _ := io.ReadAll(file)
		fmt.Println(name, len(b))
	}
}

func main() {
	if len(os.Args) > 1 {
		fmt.Println("child:", os.Getpid(), os.Getppid())
		child()
		return
	}

	fmt.Println("parent:", os.Getpid())
	parent()
}