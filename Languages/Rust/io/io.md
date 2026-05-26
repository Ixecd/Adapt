## Rust 文件与 IO

### 接收命令行参数

命令行程序是计算机程序最基础的存在形式，几乎所有的操作系统都支持命令行程序并将可视化程序的运行基于命令行机制。

命令行程序必须能够接收来自命令行环境的参数，这些参数往往在一条命令行的命令之后以空格符分隔。

在很多语言中（如 Java 和 C/C++）环境参数是以主函数的参数（常常是一个字符串数组）传递给程序的，但在 Rust 中主函数是个无参函数，环境参数需要开发者通过 std::env 模块取出，过程十分简单：

实例

```rust
fn main() {
    let args = std::env::args();
    println!("{:?}", args);
}
```
现在直接运行程序：

```bash
Args { inner: ["D:\\rust\\greeting\\target\\debug\\greeting.exe"] }
```

也许你得到的结果比这个要长的多，这很正常，这个结果中 Args 结构体中有一个 inner 数组，只包含唯一的字符串，代表了当前运行的程序所在的位置。

但这个数据结构令人难以理解，没关系，我们可以简单地遍历它：

实例
```rust
fn main() {
    let args = std::env::args();
    for arg in args {
        println!("{}", arg);
    }
}
```
运行结果：
```bash
D:\rust\greeting\target\debug\greeting.exe
```
一般参数们就是用来被遍历的，不是吗。

现在我们打开许久未碰的 launch.json ，找到 "args": []，这里可以设置运行时的参数，我们将它写成 "args": ["first", "second"] ，然后保存、再次运行刚才的程序，运行结果：

```bash
D:\rust\greeting\target\debug\greeting.exe
first
second
```

作为一个真正的命令行程序，我们从未真正使用过它，作为语言教程不在此叙述如何用命令行运行 Rust 程序。但如果你是个训练有素的开发者，你应该可以找到可执行文件的位置，你可以尝试进入目录并使用命令行命令来测试程序接收命令行环境参数。

### 命令行输入

早期的章节详细讲述了如何使用命令行输出，这是由于语言学习的需要，没有输出是无法调试程序的。但从命令行获取输入的信息对于一个命令行程序来说依然是相当重要的。

在 Rust 中，std::io 模块提供了标准输入（可认为是命令行输入）的相关功能：

实例

```rust
use std::io::stdin;

fn main() {
let mut str_buf = String::new();

    stdin().read_line(&mut str_buf)
        .expect("Failed to read line.");

    println!("Your input line is \n{}", str_buf);
}
```

令 VSCode 环境支持命令行输入是一个非常繁琐的事情，牵扯到跨平台的问题和不可调试的问题，所以我们直接在 VSCode 终端中运行程序。在命令行中运行：

```bash
D:\rust\greeting> cd ./target/debug
D:\rust\greeting\target\debug> ./greeting.exe
RUNOOB
Your input line is 
RUNOOB
```

std::io::Stdio 包含 read_line 读取方法，可以读取一行字符串到缓冲区，返回值都是 Result 枚举类，用于传递读取中出现的错误，所以常用 expect 或 unwrap 函数来处理错误。

**注意：**目前 Rust 标准库还没有提供直接从命令行读取数字或格式化数据的方法，我们可以读取一行字符串并使用字符串识别函数处理数据。

### 文件读取

我们在计算机的 D:\ 目录下建立文件 text.txt，内容如下：

```txt
This is a text file.
```

这是一个将文本文件内容读入字符串的程序：

实例

```rust
use std::fs;

fn main() {
    let text = fs::read_to_string("D:\\text.txt").unwrap();
    println!("{}", text);
}
```

运行结果：

```bash
This is a text file.
```

在 Rust 中读取内存可容纳的一整个文件是一件极度简单的事情，std::fs 模块中的 read_to_string 方法可以轻松完成文本文件的读取。

但如果要读取的文件是二进制文件，我们可以用 std::fs::read 函数读取 u8 类型集合：

实例
```rust
use std::fs;

fn main() {
    let content = fs::read("D:\\text.txt").unwrap();
    println!("{:?}", content);
}
```

运行结果：

```bash
[84, 104, 105, 115, 32, 105, 115, 32, 97, 32, 116, 101, 120, 116, 32, 102, 105, 108, 101, 46]
```

以上两种方式是一次性读取，十分适合 Web 应用的开发。但是对于一些底层程序来说，传统的按流读取的方式依然是无法被取代的，因为更多情况下文件的大小可能远超内存容量。

Rust 中的文件流读取方式：

实例
```rust
use std::io::prelude::*;
use std::fs;

fn main() {
    let mut buffer = [0u8; 5];
    let mut file = fs::File::open("D:\\text.txt").unwrap();
    file.read(&mut buffer).unwrap();
    println!("{:?}", buffer);
    file.read(&mut buffer).unwrap();
    println!("{:?}", buffer);
}
```
运行结果：

```bash
[84, 104, 105, 115, 32] 
[105, 115, 32, 97, 32]
```

std::fs 模块中的 File 类是描述文件的类，可以用于打开文件，再打开文件之后，我们可以使用 File 的 read 方法按流读取文件的下面一些字节到缓冲区（缓冲区是一个 u8 数组），读取的字节数等于缓冲区的长度。

注意：VSCode 目前还不具备自动添加标准库引用的功能，所以有时出现"函数或方法不存在"一样的错误有可能是标准库引用的问题。我们可以查看标准库的注释文档（鼠标放到上面会出现）来手动添加标准库。

std::fs::File 的 open 方法是"只读"打开文件，并且没有配套的 close 方法，因为 Rust 编译器可以在文件不再被使用时自动关闭文件。

### 文件写入

文件写入分为一次性写入和流式写入。流式写入需要打开文件，打开方式有"新建"（create）和"追加"（append）两种。

一次性写入：

实例

```rust
use std::fs;

fn main() {
    fs::write("D:\\text.txt", "FROM RUST PROGRAM")
        .unwrap();
}
```

这和一次性读取一样简单方便。执行程序之后， D:\text.txt 文件的内容将会被重写为 FROM RUST PROGRAM 。所以，一次性写入请谨慎使用！因为它会直接删除文件内容（无论文件多么大）。如果文件不存在就会创建文件。

如果想使用流的方式写入文件内容，可以使用 std::fs::File 的 create 方法：

实例
```rust
use std::io::prelude::*;
use std::fs::File;

fn main() {
    let mut file = File::create("D:\\text.txt").unwrap();
    file.write(b"FROM RUST PROGRAM").unwrap();
}
```

这段程序与上一个程序等价。

**注意：**打开的文件一定存放在可变的变量中才能使用 File 的方法！

File 类中不存在 append 静态方法，但是我们可以使用 OpenOptions 来实现用特定方法打开文件：

实例

```rust
use std::io::prelude::*;
use std::fs::OpenOptions;

fn main() -> std::io::Result<()> {
   
    let mut file = OpenOptions::new()
            .append(true).open("D:\\text.txt")?;

    file.write(b" APPEND WORD")?;

    Ok(())
}
```

运行之后，D:\text.txt 文件内容将变成：

```txt
FROM RUST PROGRAM APPEND WORD
```

OpenOptions 是一个灵活的打开文件的方法，它可以设置打开权限，除append 权限以外还有 read 权限和 write 权限，如果我们想以读写权限打开一个文件可以这样写：

实例

```rust
use std::io::prelude::*;
use std::fs::OpenOptions;

fn main() -> std::io::Result<()> {
   
    let mut file = OpenOptions::new()
            .read(true).write(true).open("D:\\text.txt")?;

    file.write(b"COVER")?;

    Ok(())
}
```

运行之后，D:\text.txt 文件内容将变成：

```txt
COVERRUST PROGRAM APPEND WORD
```