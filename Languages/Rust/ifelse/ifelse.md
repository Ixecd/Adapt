## Rust 条件语句

在 Rust 语言中的条件语句是这种格式的：

实例

```rust
fn main() {
    let number = 3;
    if number < 5 {
        println!("条件为 true");
    } else {
        println!("条件为 false");
    }
}
```

在上述程序中有条件 if 语句，这个语法在很多其它语言中很常见，但也有一些区别：首先，条件表达式 number < 5 不需要用小括号包括（注意，不需要不是不允许）；但是 Rust 中的 if 不存在单语句不用加 {} 的规则，不允许使用一个语句代替一个块。尽管如此，Rust 还是支持传统 else-if 语法的：

实例

```rust
fn main() {
    let a = 12;
    let b;
    if a > 0 {
        b = 1;
    }  
    else if a < 0 {
        b = -1;
    }  
    else {
        b = 0;
    }
    println!("b is {}", b);
}
```

运行结果：

```bash
b 为 1
```

Rust 中的条件表达式必须是 bool 类型，例如下面的程序是错误的：

实例

```rust
fn main() {
    let number = 3;
    if number {   // 报错，expected `bool`, found integerrustc(E0308)
        println!("Yes");
    }
}
```

虽然 C/C++ 语言中的条件表达式用整数表示，非 0 即真，但这个规则在很多注重代码安全性的语言中是被禁止的。

```txt
if <condition> { block 1 } else { block 2 } 
```

这种语法中的 `{ block 1 }` 和 `{ block 2 }` 可不可以是函数体表达式呢？

答案是肯定的！也就是说，在 Rust 中我们可以使用 if-else 结构实现类似于三元条件运算表达式 `(A ? B : C)` 的效果：

实例

```rust
fn main() {
    let a = 3;
    let number = if a > 0 { 1 } else { -1 };
    println!("number 为 {}", number);
}
```

运行结果：

```bash
number 为 1
```

注意：两个函数体表达式的类型必须一样！且必须有一个 else 及其后的表达式块。