## Rust 集合与字符串

集合（Collection）是数据结构中最普遍的数据存放形式，Rust 标准库中提供了丰富的集合类型帮助开发者处理数据结构的操作。

### 向量

向量（Vector）是一个存放多值的单数据结构，该结构将相同类型的值线性的存放在内存中。

向量是线性表，在 Rust 中的表示是 Vec<T>。

向量的使用方式类似于列表（List），我们可以通过这种方式创建指定类型的向量：

```rust
let vector: Vec<i32> = Vec::new(); // 创建类型为 i32 的空向量
let vector = vec![1, 2, 4, 8];     // 通过数组创建向量
```

我们使用线性表常常会用到追加的操作，但是追加和栈的 push 操作本质是一样的，所以向量只有 push 方法来追加单个元素：

实例

```rust
fn main() {
    let mut vector = vec![1, 2, 4, 8];
    vector.push(16);
    vector.push(32);
    vector.push(64);
    println!("{:?}", vector);
}
```

运行结果：

```bash
[1, 2, 4, 8, 16, 32, 64]
```

append 方法用于将一个向量拼接到另一个向量的尾部：

实例

```rust
fn main() {
    let mut v1: Vec<i32> = vec![1, 2, 4, 8];
    let mut v2: Vec<i32> = vec![16, 32, 64];
    v1.append(&mut v2);
    println!("{:?}", v1);
}
```

运行结果：

```bash
[1, 2, 4, 8, 16, 32, 64]
```

get 方法用于取出向量中的值：

实例

```rust
fn main() {
    let mut v = vec![1, 2, 4, 8];
    println!("{}", match v.get(0) {
        Some(value) => value.to_string(),
        None => "None".to_string()
    });
}
```

运行结果：

```bash
1
```

因为向量的长度无法从逻辑上推断，get 方法无法保证一定取到值，所以 get 方法的返回值是 Option 枚举类，有可能为空。

这是一种安全的取值方法，但是书写起来有些麻烦。如果你能够保证取值的下标不会超出向量下标取值范围，你也可以使用数组取值语法：

实例

```rust
fn main() {
    let v = vec![1, 2, 4, 8];
    println!("{}", v[1]);
}
```

运行结果：

```bash
2
```

但如果我们尝试获取 v[4] ，那么向量会返回错误。

遍历向量：

实例

```rust
fn main() {
    let v = vec![100, 32, 57];
    for i in &v {
            println!("{}", i);
    }
}
```

运行结果：

```bash
100
32
57
```

如果遍历过程中需要更改变量的值：

实例

```rust
fn main() {
    let mut v = vec![100, 32, 57];
    for i in &mut v {
        *i += 50;
    }
}
```

### 字符串

本章主要介绍字符串的方法和 UTF-8 性质。

新建字符串：

```rust
let string = String::new();
```

基础类型转换成字符串：

```rust
let one = 1.to_string();         // 整数到字符串
let float = 1.3.to_string();     // 浮点数到字符串
let slice = "slice".to_string(); // 字符串切片到字符串
```

包含 UTF-8 字符的字符串：

```rust
let hello = String::from("السلام عليكم");
let hello = String::from("Dobrý den");
let hello = String::from("Hello");
let hello = String::from("שָׁלוֹם");
let hello = String::from("नमस्ते");
let hello = String::from("こんにちは");
let hello = String::from("안녕하세요");
let hello = String::from("你好");
let hello = String::from("Olá");
let hello = String::from("Здравствуйте");
let hello = String::from("Hola");
```

字符串追加：

```rust
let mut s = String::from("run");
s.push_str("oob"); // 追加字符串切片
s.push('!');       // 追加字符
```

用 + 号拼接字符串：

```rust
let s1 = String::from("Hello, ");
let s2 = String::from("world!");
let s3 = s1 + &s2;
```

这个语法也可以包含字符串切片：

```rust
let s1 = String::from("tic");
let s2 = String::from("tac");
let s3 = String::from("toe");

let s = s1 + "-" + &s2 + "-" + &s3;
```

使用 format! 宏：

```rust
let s1 = String::from("tic");
let s2 = String::from("tac");
let s3 = String::from("toe");

let s = format!("{}-{}-{}", s1, s2, s3);
```

字符串长度：

```rust
let s = "hello";
let len = s.len();
```

这里 len 的值是 5。

```rust
let s = "你好";
let len = s.len();
```

这里 len 的值是 6。因为中文是 UTF-8 编码的，每个字符长 3 字节，所以长度为6。但是 Rust 中支持 UTF-8 字符对象，所以如果想统计字符数量可以先取字符串为字符集合：

```rust
let s = "hello你好";
let len = s.chars().count();
```

这里 len 的值是 7，因为一共有 7 个字符。统计字符的速度比统计数据长度的速度慢得多。

遍历字符串：

实例

```rust
fn main() {
    let s = String::from("hello中文");
    for c in s.chars() {
        println!("{}", c);
    }
}
```

运行结果：

```bash
h
e
l
l
o
中
文
```

从字符串中取单个字符：

实例

```rust
fn main() {
    let s = String::from("EN中文");
    let a = s.chars().nth(2);
    println!("{:?}", a);
}
```

运行结果：

```bash
Some('中')
```

注意：nth 函数是从迭代器中取出某值的方法，请不要在遍历中这样使用！因为 UTF-8 每个字符的长度不一定相等！

如果想截取字符串字串：
实例
fn main() {
    let s = String::from("EN中文");
    let sub = &s[0..2];
    println!("{}", sub);
}
运行结果：

```bash
EN
```

但是请注意此用法有可能肢解一个 UTF-8 字符！那样会报错：

实例

```rust
fn main() {
    let s = String::from("EN中文");
    let sub = &s[0..3];
    println!("{}", sub);
}
```

运行结果：

```bash
thread 'main' panicked at 'byte index 3 is not a char boundary; it is inside '中' (bytes 2..5) of `EN中文`', src\libcore\str\mod.rs:2069:5 
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace.
```

### 映射表

映射表（Map）在其他语言中广泛存在。其中应用最普遍的就是键值散列映射表（Hash Map）。

新建一个散列值映射表：

实例

```rust
use std::collections::HashMap;

fn main() {
    let mut map = HashMap::new();

    map.insert("color", "red");
    map.insert("size", "10 m^2");

    println!("{}", map.get("color").unwrap());
}
```

**注意：**这里没有声明散列表的泛型，是因为 Rust 的自动判断类型机制。

运行结果：

```bash
red
```

insert 方法和 get 方法是映射表最常用的两个方法。

映射表支持迭代器：

实例

```rust
use std::collections::HashMap;

fn main() {
    let mut map = HashMap::new();

    map.insert("color", "red");
    map.insert("size", "10 m^2");

    for p in map.iter() {
        println!("{:?}", p);
    }
}
```

运行结果：

```bash
("color", "red") 
("size", "10 m^2")
```

迭代元素是表示键值对的元组。

Rust 的映射表是十分方便的数据结构，当使用 insert 方法添加新的键值对的时候，如果已经存在相同的键，会直接覆盖对应的值。如果你想"安全地插入"，就是在确认当前不存在某个键时才执行的插入动作，可以这样：

```rust
map.entry("color").or_insert("red");
```

这句话的意思是如果没有键为 "color" 的键值对就添加它并设定值为 "red"，否则将跳过。

在已经确定有某个键的情况下如果想直接修改对应的值，有更快的办法：

实例

```rust
use std::collections::HashMap;

fn main() {
    let mut map = HashMap::new();
    map.insert(1, "a");
   
    if let Some(x) = map.get_mut(&1) {
        *x = "b";
    }
}
```