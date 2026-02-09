## 延迟

延迟调用被编译器转换成 deferproc 和 deferreturn 调用。不过为提升其执行性能，编译器一直在优化，比如在栈上分配，或直接内联调用

## 新建

延迟调用函数被打包成 _defer，放入 G._defer 链表内

栈分配的 _defer 对象更简单。在栈上预留空间，将指针作为参数，进行初始化

## 执行

在函数结束前插入的 deferreturn 会遍历 G._defer 链表，执行属于当前函数的延迟调用

## 恐慌

编译器将 panic 翻译成 gopanic 调用。和 defer 类似，panic也会保存在 G 里

引发恐慌前，需确保 G 所有已注册（整个调用堆栈，非当前函数）延迟函数得以执行

进程崩溃前，不会等待其他 G，不会执行其他 G._defer。恐慌代表 「不可修复错误」，如等待其他G，可能永远无法终止进程，这就违背设计初衷

## 恢复

在延迟函数内调用 recover，被编译器转换为 gorecover 调用

**执行逻辑**:

- 首先，gopanic 执行 G._defer 函数
- 接着，_defer 调用 gorecover 设置 recovered = true 标记
- 最后，recovery 获取 _defer.caller 状态，恢复执行 caller "call deferproc" 下一条指令