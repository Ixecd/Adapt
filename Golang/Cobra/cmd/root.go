package cmd

import (
	"fmt"
	"os"
	"errors"

	homedir "github.com/mitchellh/go-homedir"
	"github.com/spf13/cobra"
	"github.com/spf13/viper"
)

// 一般情况下 rootCmd 是根指令， 存放在cmd/root.go中
var (
	cfgFile     string
	projectBase string
	userLicense string
)

var rootCmd = &cobra.Command{
	Use:   "cobra",
	Short: "cobra is a CLI tool for generating Go code",
	Long: `cobra is a CLI tool for generating Go code.
It is a very powerful tool that can help you generate Go code very quickly.`,
	Args: func(cmd *cobra.Command, args []string) error {
		if (len(args) < 1) {
			return errors.New("required at least 1 non-flag argument")
		}
		if (args[0] == "version") {
			return nil
		}
		return fmt.Errorf("unknown command %q", args[0])
	},
// 当程序执行到这一步的时候， 配置文件， 命令行参数已经解析完毕， 可以根据配置文件， 命令行参数执行对应的逻辑
	Run: func(cmd *cobra.Command, args []string) {
		// 执行根指令的逻辑
		fmt.Println("hello, World")
	},
}

func Execute() {
	if err := rootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

// cobra - CLI 工具
// viper - 配置文件工具
// homedir - 家目录工具
// cobra 与 pflag结合 可以将命令行参数与配置文件参数结合起来
// 1. 持久化的标志: 意味着该标志可用于所有的子指令
// cmd.PersistentFlags().BoolVarP();
// 2. 本地的标志: 意味着该标志仅用于当前指令
// cmd.Flags().BoolVarP();

func init() {
	// 设置cobra初始化时需要执行的函数
	cobra.OnInitialize(initConfig)

	rootCmd.AddCommand(versionCmd)

	// 添加持久性的字符串标志
	rootCmd.PersistentFlags().StringVar(&cfgFile, "config", "", "config file (default is $HOME/.cobra.yaml)")
	rootCmd.PersistentFlags().StringVarP(&projectBase, "project-base", "b", "", "project base path eg github.Ixecd/spf13/")
	rootCmd.PersistentFlags().StringP("author", "a", "JOKER", "author name")
	rootCmd.PersistentFlags().StringVarP(&userLicense, "license", "l", "", "license name")
	rootCmd.PersistentFlags().Bool("viper", true, "Use Viper for configuration")
	// viper配置中的 author 绑定到命令行标志 --author，
	viper.BindPFlag("author", rootCmd.PersistentFlags().Lookup("author"))
	viper.BindPFlag("license", rootCmd.PersistentFlags().Lookup("license"))
	// 为 useViper 健设置默认值 true
	viper.BindPFlag("useViper", rootCmd.PersistentFlags().Lookup("viper"))
	// 为 author 健设置默认值 JOKER
	viper.SetDefault("author", "JOKER")
	viper.SetDefault("license", "MIT")
}

var versionCmd = &cobra.Command{
	Use:   "version",
	Short: "Print the version number of Cobra",
	// 如果没有至少1个非选项参数，报错
	Args: cobra.MinimumNArgs(1),
	// cobra.ExactArgs(1), 非选项参数必须为1个
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Println("Cobra Version 1.0.0")
	},
}

func initConfig() {
	// 读取配置文件
	if cfgFile != "" {
		// 使用指定的配置文件
		viper.SetConfigFile(cfgFile)
	} else {
		// 查找家目录
		home, err := homedir.Dir()
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		// 添加配置文件路径
		viper.AddConfigPath(home)
		viper.SetConfigName(".cobra")
	}
	// 读取配置文件
	if err := viper.ReadInConfig(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}
