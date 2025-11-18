package main

import (
    "fmt"
    "github.com/spf13/cobra"
)

var rootCmd = &cobra.Command{
    Use:   "greet",
    Short: "打招呼工具",
}

var greetCmd = &cobra.Command{
    Use:   "hello",
    Short: "说你好",
    Run: func(cmd *cobra.Command, args []string) {
        name, _ := cmd.Flags().GetString("name")
        times, _ := cmd.Flags().GetInt("times")
        
        for i := 0; i < times; i++ {
            fmt.Printf("你好, %s!\n", name)
        }
    },
}

func init() {
    // 定义参数
    greetCmd.Flags().StringP("name", "n", "世界", "要打招呼的名字")
    greetCmd.Flags().IntP("times", "t", 1, "重复次数")
    
    rootCmd.AddCommand(greetCmd)
}

func main() {
    rootCmd.Execute()
}
