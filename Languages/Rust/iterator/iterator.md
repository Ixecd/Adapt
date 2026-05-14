## Rust 迭代器

Rust 中的迭代器（Iterator）是一个强大且灵活的工具，用于对集合（如数组、向量、链表等）进行逐步访问和操作。

Rust 的迭代器是惰性求值的，这意味着迭代器本身不会立即执行操作，而是在需要时才会产生值。

迭代器允许以一种声明式的方式来遍历序列，如数组、切片、链表等集合类型的元素。

迭代器背后的核心思想是将数据处理过程与数据本身分离，使代码更清晰、更易读、更易维护。

在 Rust 中，迭代器通过实现 Iterator trait 来定义。

最基本的 trait 方法是 next，用于逐一返回迭代器中的下一个元素，直到返回 None 表示结束。

实例
```rust
pub trait Iterator {
    type Item;
   
    fn next(&mut self) -> Option<Self::Item>;
   
    // 其他默认实现的方法如 map, filter 等。
}
```

**迭代器遵循以下原则：**

- **惰性求值 (Laziness)**：Rust 中的迭代器是惰性的，意味着迭代器本身不会立即进行任何计算或操作，直到你显式地请求数据。这使得迭代器在性能上表现良好，可以避免不必要的计算。

- **所有权和借用检查 (Ownership and Borrowing Checks)**：Rust 迭代器严格遵守所有权和借用规则，避免数据竞争和内存错误。迭代器的生命周期与底层数据相关联，确保数据的安全访问。

- **链式调用 (Chaining)**：Rust 迭代器支持链式调用，即可以将多个迭代器方法链接在一起进行组合操作，这使得代码简洁且具有高度可读性。例如，通过使用 .map()、.filter()、.collect() 等方法，可以创建复杂的数据处理流水线。

- **高效内存管理 (Efficient Memory Management)**：迭代器避免了不必要的内存分配，因为大多数操作都是惰性求值的，并且在使用时直接进行遍历操作。这对于处理大数据集合尤其重要。

- **抽象和通用性 (Abstraction and Generality)**：Rust 的迭代器通过 Iterator trait 实现抽象和通用性。任何实现了 Iterator trait 的类型都可以在不同的上下文中作为迭代器使用。此设计提高了代码的重用性和模块化。

### 创建迭代器

最常见的方式是通过集合的 .iter()、.iter_mut() 或 .into_iter() 方法来创建迭代器：

- .iter()：返回集合的不可变引用迭代器。
- .iter_mut()：返回集合的可变引用迭代器。
- .into_iter()：将集合转移所有权并生成值迭代器。

使用 iter() 方法创建借用迭代器：

```rust
let vec = vec![1, 2, 3, 4, 5];
let iter = vec.iter();
```

使用 iter_mut() 方法创建可变借用迭代器：

```rust
let mut vec = vec![1, 2, 3, 4, 5];
let iter_mut = vec.iter_mut();
```

使用 into_iter() 方法创建获取所有权的迭代器：

```rust
let vec = vec![1, 2, 3, 4, 5];
let into_iter = vec.into_iter();
```

实例
```rust
let v = vec![1, 2, 3];
let mut iter = v.iter();

assert_eq!(iter.next(), Some(&1));
assert_eq!(iter.next(), Some(&2));
assert_eq!(iter.next(), Some(&3));
assert_eq!(iter.next(), None);  // 迭代结束
```

### 迭代器方法
Rust 的迭代器提供了丰富的方法来处理集合中的元素，其中一些常见的方法包括：

- map()：对每个元素应用给定的转换函数。
- filter()：根据给定的条件过滤集合中的元素。
- fold()：对集合中的元素进行累积处理。
- skip()：跳过指定数量的元素。
- take()：获取指定数量的元素。
- enumerate()：为每个元素提供索引。
- ......
使用 map() 方法对每个元素进行转换：

```rust
let vec = vec![1, 2, 3, 4, 5];
let squared_vec: Vec<i32> = vec.iter().map(|x| x * x).collect();
```

使用 filter() 方法根据条件过滤元素：

```rust
let vec = vec![1, 2, 3, 4, 5];
let filtered_vec: Vec<i32> = vec.into_iter().filter(|&x| x % 2 == 0).collect();
```

### 使用 for 循环遍历迭代器

Rust 提供了 for 循环语法来遍历迭代器中的元素，是一种更加简洁和直观的遍历方式。

Rust 的 for 循环底层实际上是使用迭代器的。

```rust
let vec = vec![1, 2, 3, 4, 5];
for &num in vec.iter() {
    println!("{}", num);
}
```

在这个循环中，vec.iter() 返回一个迭代器，for 循环遍历这个迭代器，并将每个元素赋值给 num 变量，然后执行循环体中的代码。

### 消耗型适配器

使用迭代器直到它被完全消耗。

迭代器有许多可以消耗迭代器的方法，它们会通过执行迭代来返回最终结果（比如总和、集合等），这些方法会消耗迭代器本身。

- collect()：将迭代器转换为集合（如向量、哈希集）。
- sum()：计算迭代器中所有元素的和。
- product()：计算迭代器中所有元素的乘积。
- count()：返回迭代器中元素的个数。

实例
```rust
let v = vec![1, 2, 3];
let sum: i32 = v.iter().sum();
assert_eq!(sum, 6);
```

### 适配器

迭代器适配器允许你通过方法链来改变或过滤迭代器的内容，而不会立刻消耗它。

- map()：对每个元素应用某个函数，并返回一个新的迭代器。
- filter()：过滤出满足条件的元素。
- take(n)：只返回前 n 个元素的迭代器。
- skip(n)：跳过前 n 个元素，返回剩下的元素迭代器。
```rust
let v = vec![1, 2, 3, 4, 5];
let doubled: Vec<i32> = v.iter().map(|x| x * 2).collect();
assert_eq!(doubled, vec![2, 4, 6, 8, 10]);
```

### 迭代器链

可以将多个迭代器适配器链接在一起，形成迭代器链。

实例
```rust
use std::iter::Peekable;

let arr = [1, 2, 3, 4, 5];
let mut iter = arr.into_iter().peekable();
while let Some(val) = iter.next() {
    if val % 2 == 0 {
        continue;
    }
    println!("{}", val);
}
```

### 收集器

使用 collect 方法将迭代器的元素收集到某种集合中。
```rust
let arr = [1, 2, 3, 4, 5];
let sum: i32 = arr.into_iter().sum();
```

### 惰性求值

正如前面提到的，Rust 迭代器是惰性的，这意味着像 map()、filter() 等不会立刻执行操作，直到调用像 collect() 这样的消耗性方法才会真正处理数据。这使得迭代器处理更加高效，因为避免了不必要的计算。

### 自定义迭代器

也可以为自己的类型实现 Iterator trait，只需定义 next() 方法即可。

例如，实现一个从 1 到 5 的简单迭代器：

实例
```rust
struct Counter {
    count: usize,
}

impl Counter {
    fn new() -> Counter {
        Counter { count: 0 }
    }
}

impl Iterator for Counter {
    type Item = usize;
   
    fn next(&mut self) -> Option<Self::Item> {
        self.count += 1;
        if self.count <= 5 {
            Some(self.count)
        } else {
            None
        }
    }
}

let mut counter = Counter::new();
while let Some(num) = counter.next() {
    println!("{}", num);  // 输出 1 到 5
}
```

### 并行迭代器

如果需要在多线程环境中并行化操作，rayon crate 提供了并行迭代器的支持，通过 .par_iter() 代替 .iter()，可以在多线程环境中加速迭代操作。

### 迭代器和生命周期

迭代器的生命周期与它所迭代的元素的生命周期相关联。迭代器可以借用元素，也可以取得元素的所有权。这在迭代器的实现中通过生命周期参数来控制。

### 迭代器与闭包

迭代器适配器经常与闭包一起使用，闭包允许你为迭代器操作提供定制逻辑。

### 迭代器和性能

迭代器通常是非常高效的，因为它们允许编译器做出优化。例如，编译器可以内联迭代器适配器的调用，并且可以利用迭代器的惰性求值特性。

实例
下面实例演示了如何使用迭代器对一个数组进行遍历，并输出数组中的元素。

实例
```rust
// 主函数
fn main() {
    // 定义一个包含整数的数组
    let numbers = vec![1, 2, 3, 4, 5];

    // 使用迭代器对数组进行遍历，并输出每个元素
    println!("Iterating through the array:");
    for num in numbers.iter() {
        println!("{}", num);
    }

    // 使用迭代器的 map 方法对数组中的每个元素进行平方运算，并收集结果到一个新的数组中
    let squared_numbers: Vec<i32> = numbers.iter().map(|x| x * x).collect();

    // 输出平方后的数组
    println!("Squared numbers: {:?}", squared_numbers);
}
```
以上代码中，我们首先定义了一个包含整数的数组 numbers，然后使用 iter() 方法获取数组的迭代器，并通过 for 循环遍历迭代器，输出数组中的每个元素。接着使用迭代器的 map() 方法对数组中的每个元素进行平方运算，并使用 collect() 方法将结果收集到一个新的数组 squared_numbers 中。最后输出了平方后的数组。

运行该程序，可以看到输出了原始数组中的每个元素，以及经过平方运算后的新数组：
```txt
Iterating through the array:
1
2
3
4
5
Squared numbers: [1, 4, 9, 16, 25]
```
这个例子演示了 Rust 中迭代器的基本用法，包括遍历、转换和收集结果。

以下实例使用 filter() 方法对一个数组进行过滤，并输出过滤后的结果：
```rust
实例
// 主函数
fn main() {
    // 定义一个包含整数的数组
    let numbers = vec![1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

    // 使用迭代器的 filter 方法对数组进行过滤，筛选出偶数
    let even_numbers: Vec<i32> = numbers.iter().filter(|&x| x % 2 == 0).cloned().collect();

    // 输出筛选后的结果
    println!("Even numbers: {:?}", even_numbers);
}
```
以上代码中，我们首先定义了一个包含整数的数组 numbers，然后使用迭代器的 filter() 方法对数组进行过滤，筛选出其中的偶数。在 filter() 方法的闭包中，我们使用模运算来判断元素是否为偶数。最后使用 cloned() 方法来克隆每个偶数的值，并使用 collect() 方法将结果收集到一个新的数组 even_numbers 中。最终输出了筛选后的结果。

运行该程序，可以看到输出了数组中的所有偶数：
```text
Even numbers: [2, 4, 6, 8, 10]
```
这个例子演示了 Rust 中迭代器的 filter() 方法的使用，以及如何结合其他方法来实现对数组的筛选操作。

### Rust 迭代器方法
以下是一些 Rust 中常用的迭代器方法，以及它们的简要说明和示例：

| 方法名 | 描述 | 示例
|---|---|---|
| next() | 返回迭代器中的下一个元素。 | let mut iter = (1..5).into_iter(); while let Some(val) = iter.next() { println!("{}", val); }
| size_hint() | 返回迭代器中剩余元素数量的下界和上界。 | let iter = (1..10).into_iter(); println!("{:?}", iter.size_hint());
| count() | 计算迭代器中的元素数量。 | let count = (1..10).into_iter().count();
| nth() | 返回迭代器中第 n 个元素。 | let third = (0..10).into_iter().nth(2);
| last() | 返回迭代器中的最后一个元素。 | let last = (1..5).into_iter().last();
| all() | 如果迭代器中的所有元素都满足某个条件，返回 true。 | let all_positive = (1..=5).into_iter().all(|x| x > 0);
| any() | 如果迭代器中的至少一个元素满足某个条件，返回 true。 | let any_negative = (1..5).into_iter().any(|x| x < 0);
| find() | 返回迭代器中第一个满足某个条件的元素。 | let first_even = (1..10).into_iter().find(|x| x % 2 == 0);
| find_map() | 对迭代器中的元素应用一个函数，返回第一个返回 Some 的结果。 | let first_letter = "hello".chars().find_map(|c| if c.is_alphabetic() { Some(c) } else { None });
| map() | 对迭代器中的每个元素应用一个函数。 | let squares: Vec<i32> = (1..5).into_iter().map(|x| x * x).collect();
| filter() | 保留迭代器中满足某个条件的元素。 | let evens: Vec<i32> = (1..10).into_iter().filter(|x| x % 2 == 0).collect();
| filter_map() | 对迭代器中的元素应用一个函数，如果函数返回 Some，则保留结果。 | let chars: Vec<char> = "hello".chars().filter_map(|c| if c.is_alphabetic() { Some(c.to_ascii_uppercase()) } else { None }).collect();
| map_while() | 对迭代器中的元素应用一个函数，直到函数返回 None。 | let first_three = (1..).into_iter().map_while(|x| if x <= 3 { Some(x) } else { None });
| take_while() | 从迭代器中取出满足某个条件的元素，直到不满足为止。 | let first_five = (1..10).into_iter().take_while(|x| x <= 5).collect::<Vec<_>>()
| skip_while() | 跳过迭代器中满足某个条件的元素，直到不满足为止。 | let odds: Vec<i32> = (1..10).into_iter().skip_while(|x| x % 2 == 0).collect();
| for_each() | 对迭代器中的每个元素执行某种操作。 | let mut counter = 0; (1..5).into_iter().for_each(|x| counter += x);
| fold() | 对迭代器中的元素进行折叠，使用一个累加器。 | let sum: i32 = (1..5).into_iter().fold(0, |acc, x| acc + x);
| try_fold() | 对迭代器中的元素进行折叠，可能在遇到错误时提前返回。 | let result: Result = (1..5).into_iter().try_fold(0, |acc, x| if x == 3 { Err("Found the number 3") } else { Ok(acc + x) });
| scan() | 对迭代器中的元素进行状态化的折叠。 | let sum: Vec<i32> = (1..5).into_iter().scan(0, |acc, x| { *acc += x; Some(*acc) }).collect();
| take() | 从迭代器中取出最多 n 个元素。 | let first_five = (1..10).into_iter().take(5).collect::<Vec<_>>()
| skip() | 跳过迭代器中的前 n 个元素。 | let after_five = (1..10).into_iter().skip(5).collect::<Vec<_>>()
| zip() | 将两个迭代器中的元素打包成元组。 | let zipped = (1..3).zip(&['a', 'b', 'c']).collect::<Vec<_>>()
| cycle() | 重复迭代器中的元素，直到无穷。 | let repeated = (1..3).into_iter().cycle().take(7).collect::<Vec<_>>()
| chain() | 连接多个迭代器。 | let combined = (1..3).chain(4..6).collect::<Vec<_>>()
| rev() | 反转迭代器中的元素顺序。 | let reversed = (1..4).into_iter().rev().collect::<Vec<_>>()
| enumerate() | 为迭代器中的每个元素添加索引。 | let enumerated = (1..4).into_iter().enumerate().collect::<Vec<_>>()
| peeking_take_while() | 取出满足条件的元素，同时保留迭代器的状态，可以继续取出后续元素。 | let (first, rest) = (1..10).into_iter().peeking_take_while(|&x| x < 5);
| step_by() | 按照指定的步长返回迭代器中的元素。 | let even_numbers = (0..10).into_iter().step_by(2).collect::<Vec<_>>()
| fuse() | 创建一个额外的迭代器，它在迭代器耗尽后仍然可以调用 next() 方法。 | let mut iter = (1..5).into_iter().fuse(); while iter.next().is_some() {}
| inspect() | 在取出每个元素时执行一个闭包，但不改变元素。 | let mut counter = 0; (1..5).into_iter().inspect(|x| println!("Inspecting: {}", x)).for_each(|x| println!("Processing: {}", x));
| same_items() | 比较两个迭代器是否产生相同的元素序列。 | let equal = (1..5).into_iter().same_items((1..5).into_iter());

### 总结

Rust 的迭代器是一个功能强大且灵活的工具，它允许以声明式的方式处理序列。迭代器的设计考虑了安全性、性能和表达力，是 Rust 语言的核心特性之一。通过迭代器，Rust 程序员可以写出既安全又高效的代码。