## log

轻量级日志记录器，默认输出到`stderr`，可通过`SetOutput`改变输出目标

- Print
- Panic: Print + panic
- Fatal: Print + os.Exit(1)

```go
func main() {
	l := log.New(os.Stdout, "[demo] ", log.Ldate | log.Lshortfile)
	l.Println("hello")
}
```

