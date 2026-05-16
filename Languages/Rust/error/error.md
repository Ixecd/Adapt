## Rust 错误处理

Rust 采用了一种独特的错误处理机制，**没有异常（Exception），没有 try/catch**。它将错误分为两类，用不同的方式处理：

- 不可恢复错误（Unrecoverable）：程序逻辑出了严重问题，使用 panic! 宏终止程序
- 可恢复错误（Recoverable）：操作可能失败但可以处理，使用 Result<T, E> 枚举

这种设计迫使开发者在编译期就处理可能的错误，而不是在运行时才发现遗漏。

## 1. 不可恢复错误：panic!

panic! 宏用于表示程序遇到了无法继续执行的严重错误。调用时会：

1. 打印错误信息和发生位置

2. 展开（unwind）调用栈并清理资源

3. 终止程序

实例

```rust
fn main() {
    panic!("发生了严重错误");
    // 下面的代码永远不会执行
    println!("Hello, Rust");
}
```

运行结果：

```bash
thread 'main' panicked at '发生了严重错误', src/main.rs:2:5
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace.
```

输出的两行信息：

- 第一行：panic 的位置（文件名和行号）以及错误信息
- 第二行：提示如何查看完整的调用栈回溯

### 查看调用栈回溯（Backtrace）

设置 RUST_BACKTRACE=1 环境变量可以看到完整的调用栈，帮助定位 panic 的根源：

```bash
# Linux / macOS
RUST_BACKTRACE=1 cargo run

# Windows PowerShell
$env:RUST_BACKTRACE=1; cargo run
```

回溯输出会列出从 panic 发生位置到 main 函数的完整调用链。当 panic 发生在深层调用中时，这个信息非常有用。

> 何时使用 panic!：代码中出现了不应该发生的逻辑错误（bug），比如访问越界、解引用空指针、违反了不可变的契约。对于外部输入导致的可预期错误，应使用 Result。

## 2. 可恢复错误：Result<T, E>

Result 是 Rust 标准库中用于表示可能失败的操作的枚举：

```rust
enum Result&lt;T, E&gt; {
    Ok(T),    // 操作成功，包含结果值
    Err(E),   // 操作失败，包含错误信息
}
```

### 2.1 使用 match 处理 Result

实例

```rust
use std::fs::File;

fn main() {
    let f = File::open("hello.txt");

    match f {
        Ok(file) => {
            println!("文件打开成功: {:?}", file);
        }
        Err(error) => {
            println!("文件打开失败: {}", error);
        }
    }
}
```

### 2.2 使用 if let 简化处理

当你只关心成功的情况时，if let 比 match 更简洁：

实例

```rust
use std::fs::File;

fn main() {
    let f = File::open("hello.txt");

    if let Ok(file) = f {
        println!("文件打开成功");
        // 在这里使用 file ...
    } else {
        println!("文件打开失败");
    }
}
```

### 2.3 unwrap 和 expect：快速但危险

如果你确定操作不会失败（或在原型阶段不想处理错误），可以用这两个快捷方法：

| 方法 | 行为 | 失败时的 panic 信息
|---|---|---|
| .unwrap() | 成功返回 T，失败直接 panic! | 使用默认的错误信息
| .expect("msg") | 成功返回 T，失败直接 panic! | 使用自定义的错误信息（更易调试）

实例

```rust
use std::fs::File;

fn main() {
    // unwrap：失败时 panic，使用默认信息
    // thread 'main' panicked at 'called `Result::unwrap()` on an `Err` value: ...'
    let f1 = File::open("hello.txt").unwrap();

    // expect：失败时 panic，使用自定义信息（推荐）
    // thread 'main' panicked at '无法打开配置文件: ...'
    let f2 = File::open("hello.txt").expect("无法打开配置文件");
}
```

> 建议：生产代码中优先使用 expect 而非 unwrap，因为自定义的错误信息能让你在 panic 时快速定位问题。更好的做法是使用 ? 运算符将错误传播给调用者。

## 3. 错误传播：? 运算符

在实际开发中，函数遇到错误时往往不想自己处理，而是将错误**传播**给调用者。Rust 提供了 ? 运算符来简化这一操作。

### 3.1 手动传播 vs ? 运算符

先看手动传播的写法——冗长但清晰：

实例

```rust
use std::fs::File;
use std::io::{self, Read};

// 手动传播错误（繁琐）
fn read_file_manual(path: &str) -> Result<String, io::Error> {
    let f = File::open(path);

    // 打开失败则返回 Err
    let mut file = match f {
        Ok(file) => file,
        Err(e) => return Err(e),  // 提前返回错误
    };

    let mut content = String::new();

    // 读取失败则返回 Err
    match file.read_to_string(&mut content) {
        Ok(_) => Ok(content),
        Err(e) => Err(e),
    }
}
```

使用 ? 运算符，同样的逻辑可以简化为：

实例

```rust
use std::fs::File;
use std::io::{self, Read};

// 使用 ? 运算符（简洁）
fn read_file(path: &str) -> Result<String, io::Error> {
    let mut file = File::open(path)?;   // 失败自动返回 Err
    let mut content = String::new();
    file.read_to_string(&mut content)?; // 失败自动返回 Err
    Ok(content)
}
```

还可以链式调用，进一步简化：

```rust
fn read_file(path: &str) -> Result&lt;String, io::Error&gt; {
    let mut content = String::new();
    File::open(path)?.read_to_string(&mut content)?;
    Ok(content)
}
```

? 运算符的工作原理：

> 重要限制：? 运算符只能用在返回 Result（或 Option）的函数中。从 Rust 1.39 起，main 函数也可以返回 Result。

### 3.2 在 main 中使用 ?

默认的 main 函数返回 ()，不能使用 ?。但可以让 main 返回 Result：

实例
```rust
use std::fs::File;
use std::io::{self, Read};

fn read_file(path: &str) -> Result<String, io::Error> {
    let mut content = String::new();
    File::open(path)?.read_to_string(&mut content)?;
    Ok(content)
}

// main 返回 Result，这样就能在 main 中使用 ? 了
fn main() -> Result<(), Box<dyn std::error::Error>> {
    let content = read_file("hello.txt")?;  // ? 在 main 中也能用了
    println!("{}", content);
    Ok(())
}
```

## 4. 自定义错误类型与分类处理

在实际项目中，你通常需要根据不同的错误类型做不同的处理。Rust 通过 kind() 方法实现这一点：

实例

```rust
use std::fs::File;
use std::io::{self, Read};

// 将文件读取封装为独立函数，用 ? 传播错误
fn read_text_from_file(path: &str) -> Result<String, io::Error> {
    let mut f = File::open(path)?;
    let mut s = String::new();
    f.read_to_string(&mut s)?;
    Ok(s)
}

fn main() {
    match read_text_from_file("hello.txt") {
        Ok(content) => println!("文件内容:\n{}", content),
        Err(e) => {
            // 根据错误类型做不同处理
            match e.kind() {
                io::ErrorKind::NotFound => {
                    println!("文件不存在，请检查路径");
                }
                io::ErrorKind::PermissionDenied => {
                    println!("没有权限读取该文件");
                }
                _ => {
                    println!("读取文件时发生错误: {}", e);
                }
            }
        }
    }
}
```

运行结果（当文件不存在时）：

```bash
文件不存在，请检查路径
```

io::ErrorKind 常用的变体：

| ErrorKind | 含义
|---|---|
| NotFound | 文件或目录不存在
| PermissionDenied | 权限不足
| AlreadyExists | 文件已存在（创建时）
| ConnectionRefused | 连接被拒绝
| TimedOut | 操作超时
| InvalidInput | 参数无效

## 5. 自定义错误类型

在项目中，你通常需要定义自己的错误类型来表示业务逻辑中的错误：

实例

```rust
use std::fmt;
use std::num::ParseIntError;

// 定义自定义错误枚举
#[derive(Debug)]
enum AppError {
    IoError(std::io::Error),
    ParseError(ParseIntError),
    CustomError(String),
}

// 实现 Display trait，用于格式化输出
impl fmt::Display for AppError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            AppError::IoError(e) => write!(f, "IO 错误: {}", e),
            AppError::ParseError(e) => write!(f, "解析错误: {}", e),
            AppError::CustomError(msg) => write!(f, "业务错误: {}", msg),
        }
    }
}

// 实现 From trait，让 ? 运算符自动转换错误类型
impl From<std::io::Error> for AppError {
    fn from(error: std::io::Error) -> Self {
        AppError::IoError(error)
    }
}

impl From<ParseIntError> for AppError {
    fn from(error: ParseIntError) -> Self {
        AppError::ParseError(error)
    }
}

// 现在可以在同一个函数中用 ? 处理不同类型的错误
fn process_config(path: &str) -> Result<i32, AppError> {
    let content = std::fs::read_to_string(path)?; // io::Error → AppError
    let value: i32 = content.trim().parse()?;      // ParseIntError → AppError

    if value < 0 {
        return Err(AppError::CustomError("配置值不能为负数".into()));
    }

    Ok(value)
}

fn main() {
    match process_config("config.txt") {
        Ok(val) => println!("配置值: {}", val),
        Err(e) => println!("错误: {}", e),
    }
}
```

> 第三方库推荐：在实际项目中，可以使用 thiserror 库自动派生 Display 和 From 实现，大幅减少样板代码。对于应用层代码，anyhow 库提供了便捷的 anyhow::Result 类型，适合快速开发。

## 6. Option<T>：值可能不存在

除了 Result，Rust 还有另一个重要的枚举用于处理"可能没有值"的情况——Option<T>：

```rust
enum Option&lt;T&gt; {
    Some(T),  // 有值
    None,     // 没有值
}
```

Option 用于替代其他语言中的 null。Rust 中没有 null，任何可能为空的值都必须用 Option 包装：

实例

```rust
fn find_user(id: u32) -> Option<String> {
    match id {
        1 => Some("Alice".to_string()),
        2 => Some("Bob".to_string()),
        _ => None, // 用户不存在
    }
}

fn main() {
    // 使用 match 处理 Option
    match find_user(1) {
        Some(name) => println!("找到用户: {}", name),
        None => println!("用户不存在"),
    }

    // 使用 if let 简化
    if let Some(name) = find_user(99) {
        println!("找到用户: {}", name);
    } else {
        println!("用户不存在");
    }

    // unwrap_or 提供默认值
    let name = find_user(99).unwrap_or("匿名用户".to_string());
    println!("用户名: {}", name);  // 匿名用户

    // ? 运算符同样适用于 Option
    let first_char = get_first_char("hello");
    println!("首字母: {:?}", first_char);  // Some('h')
}

fn get_first_char(s: &str) -> Option<char> {
    s.chars().next() // 返回 Option<char>
}
```

Option 与 Result 的对比：

| 对比项 | Option<T> | Result<T, E>
|---|---|---|
| 用途 | 值可能存在或不存在 | 操作可能成功或失败
| 成功 | Some(T) | Ok(T)
| 失败 | None（无额外信息） | Err(E)（包含错误原因）
| 典型场景 | 查找、可选字段、默认值 | 文件操作、网络请求、解析
| 转换 | ok_or(err) → Result | ok() → Option

## 小结

| 场景 | 推荐方式 | 说明
|---|---|---|
| 程序遇到不可修复的 bug | panic!("原因") | 终止程序，用于不应该发生的情况
| 操作可能失败 | 返回 Result<T, E> | 强制调用者处理错误
| 在函数内传播错误 | ? 运算符 | 失败时自动 return Err，成功时取出值
| 快速原型 / 测试 | .expect("原因") | 失败时 panic，但有清晰的错误信息
| 值可能不存在 | Option<T> | 用 Some / None 替代 null
| 按错误类型分别处理 | e.kind() | 匹配具体的错误变体
| 自定义错误类型 | 实现 Display + From | 配合 ? 实现自动转换

> Rust 的错误处理哲学：错误是类型系统的一部分，而不是控制流的例外。编译器会强制你处理每一种可能的错误情况，这让你的程序在运行时更加可靠。