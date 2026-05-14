## Rust 闭包

Rust 中的闭包是一种匿名函数，它们可以捕获并存储其环境中的变量。

闭包允许在其定义的作用域之外访问变量，并且可以在需要时将其移动或借用给闭包。

闭包在 Rust 中被广泛应用于函数式编程、并发编程和事件驱动编程等领域。

闭包在 Rust 中非常有用，因为它们提供了一种简洁的方式来编写和使用函数。

闭包在 Rust 中非常灵活，可以存储在变量中、作为参数传递，甚至作为返回值。

闭包通常用于需要短小的自定义逻辑的场景，例如迭代器、回调函数等。

### 闭包与函数的区别

| 特性 | 闭包 | 函数
|---|---|---|
| 匿名性 | 是匿名的，可存储为变量 | 有固定名称
| 环境捕获 | 可以捕获外部变量 | 不能捕获外部变量
| 定义方式 | \| \| | 参数
| 类型推导 | 参数和返回值类型可以推导 | 必须显式指定
| 存储与传递 | 可以作为变量、参数、返回值 | 同样支持

以下是 Rust 闭包的一些关键特性和用法：

### 闭包的声明

闭包的语法声明：

```rust
let closure_name = |参数列表| 表达式或语句块;
```

参数可以有类型注解，也可以省略，Rust 编译器会根据上下文推断它们。

```rust
let add_one = |x: i32| x + 1;
```

**闭包的参数和返回值：** 闭包可以有零个或多个参数，并且可以返回一个值。

```rust
let calculate = |a, b, c| a * b + c;
```

**闭包的调用：**闭包可以像函数一样被调用。

```rust
let result = calculate(1, 2, 3);
```

### 匿名函数

闭包在 Rust 中类似于匿名函数，可以在代码中以 `{}` 语法块的形式定义，使用 `||` 符号来表示参数列表，实例如下：

```rust
let add = |a, b| a + b;
println!("{}", add(2, 3)); // 输出: 5
```

在这个示例中，add 是一个闭包，接受两个参数 a 和 b，返回它们的和。

## 捕获外部变量

闭包可以捕获周围环境中的变量，这意味着它可以访问定义闭包时所在作用域中的变量。例如：

```rust
let x = 5;
let square = |num| num * x;
println!("{}", square(3)); // 输出: 15
```

以上代码中，闭包 square 捕获了外部变量 x，并在闭包体中使用了它。

闭包可以通过三种方式捕获外部变量：

- **按引用捕获**（默认行为，类似 &T）
- **按值捕获**（类似 T）
- **可变借用捕获**（类似 &mut T）
实例
```rust
fn main() {
    let mut num = 5;

    // 按引用捕获
    let print_num = || println!("num = {}", num);
    print_num(); // 输出: num = 5

    // 按值捕获
    let take_num = move || println!("num taken = {}", num);
    take_num(); // 输出: num taken = 5
    // println!("{}", num); // 若取消注释，将报错，num 所有权被转移

    // 可变借用捕获
    let mut change_num = || num += 1;
    change_num();
    println!("num after closure = {}", num); // 输出: num after closure = 6
}
```

说明：

- 闭包默认按引用捕获外部变量。
- 使用 move 关键字可以强制按值捕获，将外部变量的所有权转移到闭包内。
- 如果闭包需要修改外部变量，需显式声明为 mut 闭包。

### 移动与借用

闭包可以通过 `move` 关键字获取外部变量的所有权，或者通过借用的方式获取外部变量的引用。例如：

**借用变量：**默认情况下，闭包会借用它捕获的环境中的变量，这意味着闭包可以使用这些变量，但不能改变它们的所有权。这种情况下，闭包和外部作用域都可以使用这些变量。

例如：

实例
```rust
let x = 10;
let add_x = |y| x + y;
println!("{}", add_x(5)); // 输出 15
println!("{}", x); // 仍然可以使用 x
```

**获取所有权：**通过在闭包前添加 move 关键字，闭包会获取它捕获的环境变量的所有权。这意味着这些变量的所有权会从外部作用域转移到闭包内部，外部作用域将无法再使用这些变量。例如：

实例
```rust
let s = String::from("hello");
let print_s = move || println!("{}", s);
print_s(); // 输出 "hello"
// println!("{}", s); // 这行代码将会报错，因为 s 的所有权已经被转移给了闭包
```
通过这两种方式，Rust 提供了灵活的机制来处理闭包与外部变量之间的关系，使得在编写并发、安全的代码时更加方便。

## 闭包的特性

1. 闭包可以作为函数参数

闭包经常作为参数传递给函数，例如迭代器的 .map()、.filter() 方法：

实例

```rust
fn apply_to_value<F>(val: i32, f: F) -> i32
where
    F: Fn(i32) -> i32,
{
    f(val)
}

fn main() {
    let double = |x| x * 2;
    let result = apply_to_value(5, double);
    println!("Result: {}", result); // 输出: Result: 10
}
```

这里的 Fn 是闭包的一个特性（trait），用于表示闭包可以被调用。

2. 闭包可以作为返回值

闭包还可以作为函数的返回值。由于闭包是匿名的，我们需要使用 impl Trait 或 Box 来描述其类型。

**使用 impl Fn 返回闭包**
实例
```rust
fn make_adder(x: i32) -> impl Fn(i32) -> i32 {
    move |y| x + y
}

fn main() {
    let add_five = make_adder(5);
    println!("5 + 3 = {}", add_five(3)); // 输出: 5 + 3 = 8
}
```

**使用 Box<dyn Fn> 返回闭包**

实例
```rust
fn make_adder(x: i32) -> Box<dyn Fn(i32) -> i32> {
    Box::new(move |y| x + y)
}

fn main() {
    let add_ten = make_adder(10);
    println!("10 + 2 = {}", add_ten(2)); // 输出: 10 + 2 = 12
}
```

3. 闭包特性（Traits）

闭包根据其捕获方式自动实现了以下三个特性：

- Fn: 不需要修改捕获的变量，闭包可以多次调用。
- FnMut: 需要修改捕获的变量，闭包可以多次调用。
- FnOnce: 只需要捕获所有权，闭包只能调用一次。

实例
```rust
fn call_closure<F>(f: F)
where
    F: FnOnce(),
{
    f(); // 只调用一次
}

fn main() {
    let name = String::from("Rust");

    // 使用 move 强制捕获所有权
    let print_name = move || println!("Hello, {}!", name);

    call_closure(print_name);
    // println!("{}", name); // 若取消注释，将报错，name 的所有权已被移动
}
```
## 更多应用说明

### 迭代器中的闭包

闭包在 Rust 中经常与迭代器一起使用，用于对集合中的元素进行处理。

例如，使用 map() 方法对集合中的每个元素进行转换：

```rust
let vec = vec![1, 2, 3];
let squared_vec: Vec<i32> = vec.iter().map(|x| x * x).collect();
println!("{:?}", squared_vec); // 输出: [1, 4, 9]
```

在这个示例中，闭包 `|x| x * x` 被传递给 `map()` 方法，对集合中的每个元素进行平方操作。

### 闭包作为参数和返回值

闭包可以作为参数传递给函数，也可以作为函数的返回值。

实例
```rust
fn call_fn<F>(f: F) where F: Fn() {
    f();
}

let add = |a, b| a + b;
call_fn(move || println!("Hello from a closure!"));
```

### 闭包和错误处理

闭包可以返回 Result 或 Option 类型，并且可以处理错误。

实例

```rust
fn find_first_positive(nums: &[i32], is_positive: impl Fn(i32) -> bool) -> Option<usize> {
    nums.iter().position(|&x| is_positive(x))
}
```

### 闭包和多线程

闭包可以用于多线程编程，因为它们可以捕获并持有必要的数据。

实例

```rust
use std::thread;

let nums = vec![1, 2, 3, 4, 5];
let handles = nums.into_iter().map(|num| {
    thread::spawn(move || {
        num * 2
    })
}).collect::<Vec<_>>();

for handle in handles {
    let result = handle.join().unwrap();
    println!("Result: {}", result);
}
```

### 闭包和性能

Rust 的闭包是轻量级的，并且 Rust 的编译器会进行优化，使得闭包的调用接近于直接调用函数。

### 闭包和生命周期

闭包的生命周期与它们所捕获的变量的生命周期相关。Rust 的生命周期系统确保闭包不会比它们捕获的任何变量活得更长。

### 闭包的类型

闭包在 Rust 中是一种特殊的类型，称为 Fn、FnMut 或 FnOnce，它们分别表示不同的闭包特性：

- Fn：闭包不可变地借用其环境中的变量。
- FnMut：闭包可变地借用其环境中的变量。
- FnOnce：闭包获取其环境中的变量的所有权，只能被调用一次。

实例

下面实例定义了一个闭包，用于对给定的数字进行平方运算，并演示了闭包的使用方法。

```rust
// 定义一个函数，接受一个闭包作为参数，将闭包应用到给定的数字上
fn apply_operation<F>(num: i32, operation: F) -> i32
where
    F: Fn(i32) -> i32,
{
    operation(num)
}

// 主函数
fn main() {
    // 定义一个数字
    let num = 5;

    // 定义一个闭包，用于对数字进行平方运算
    let square = |x| x * x;

    // 调用函数，并传入闭包作为参数，对数字进行平方运算
    let result = apply_operation(num, square);

    // 输出结果
    println!("Square of {} is {}", num, result);
}
```

以上代码中，我们首先定义了一个函数 apply_operation，该函数接受一个闭包作为参数，并将闭包应用到给定的数字上。然后在 main 函数中定义了一个数字 num 和一个闭包 square，用于对数字进行平方运算。最后调用了 apply_operation 函数，并传入了数字和闭包作为参数，得到了运算结果并输出。

运行该程序，可以看到输出了数字的平方值：

```text
Square of 5 is 25
```

这个例子演示了如何使用闭包来对数据进行操作，并将闭包作为参数传递给函数使用。

## 总结

Rust 的闭包是一种强大的抽象，它们提供了一种灵活且表达力强的方式来编写函数。

闭包可以捕获环境变量，并且可以作为参数传递或作为返回值。闭包与迭代器结合使用，可以方便地实现复杂的数据处理任务。

Rust 的闭包设计考虑了安全性、性能和生命周期，是 Rust 语言的重要组成部分。