## Rust 运算符

在 Rust 中，无论是简单的数值计算、逻辑判断，还是更复杂的模式匹配和位操作，运算符都承担着核心的角色。

Rust 既支持我们熟悉的 C 系语言常见运算符，也提供了一些独特的操作符号。熟练掌握这些运算符，不仅能让代码更简洁高效，也能帮助更好地理解 Rust 的语义。

### 1、算术运算符

|运算符 | 说明 | 示例 | 结果
|---|---|---|---|
|+ |加法 | 5 + 2 | 7
| - | 减法 | 5 - 2 | 3
| * | 乘法 | 5 * 2 | 10
| / | 除法（整除） | 5 / 2 | 2 （整数）
| % | 取余 | 5 % 2 | 1

实例

```rust
fn main() {
    let a = 10;
    let b = 3;

    println!("a + b = {}", a + b);
    println!("a - b = {}", a - b);
    println!("a * b = {}", a * b);
    println!("a / b = {}", a / b);
    println!("a % b = {}", a % b);
}
```

输出：

```bash
a + b = 13
a - b = 7
a * b = 30
a / b = 3
a % b = 1
```

Rust 没有 ** 或 ^ 这样的乘方运算符（注意：^ 是按位异或），如果要做乘方，需要使用内置的 pow 或 powf 方法：

- 整数类型使用 .pow(exp: u32)
- 浮点类型使用 .powf(exp: f64)

**整数乘方**

实例

```rust
fn main() {
    let base: i32 = 2;
    let result = base.pow(3); // 2^3

    println!("2^3 = {}", result);
}
```

输出：

```bash
2^3 = 8
```

**浮点数乘方**

实例

```rust
fn main() {
    let base: f64 = 2.0;
    let result = base.powf(2.5); // 2^2.5

    println!("2^2.5 = {}", result);
}
```

输出：

```bash
2^2.5 = 5.656854249492381
```

### 2、关系（比较）运算符

| 运算符 | 说明 | 示例 | 结果
|---|---|---|---|
| == | 相等 | 5 == 5 | true
| != | 不相等 | 5 != 2 | true
| > | 大于 | 5 > 2 | true
| < | 小于 | 5 < 2 | false
| >= | 大于等于 | 5 >= 5 | true
| <= | 小于等于 | 2 <= 5 | true

实例

```rust
fn main() {
    let x = 5;
    let y = 10;

    println!("x == y : {}", x == y);
    println!("x != y : {}", x != y);
    println!("x > y  : {}", x > y);
    println!("x < y  : {}", x < y);
    println!("x >= y : {}", x >= y);
    println!("x <= y : {}", x <= y);
}
```

输出：

```bash
x == y : false
x != y : true
x > y  : false
x < y  : true
x >= y : false
x <= y : true
```

### 3、逻辑运算符

| 运算符 | 说明 | 示例 | 结果
|---|---|---|---|
| && | 逻辑与（AND） | true && false | false
| \|\| | 逻辑或（OR） | true \|\| false | true
| ! | 逻辑非（NOT） | !true | false

实例

```rust
fn main() {
    let a = true;
    let b = false;

    println!("a && b = {}", a && b);
    println!("a || b = {}", a || b);
    println!("!a = {}", !a);
}
```

输出：

```bash
a && b = false
a || b = true
!a = false
```

### 4、位运算符

| 运算符 | 说明 | 示例 | 结果
|---|---|---|---|
| & | 按位与 | 5 & 3 | 1
| \| | 按位或 | 5 \| 3 | 7
| ^ | 按位异或 | 5 ^ 3 | 6
| ! | 按位取反 | !5 | -6
| << | 左移 | 5 << 1 | 10
| >> | 右移 | 5 >> 1 | 2

实例

```rust
fn main() {
    let x: u8 = 0b1010; // 10
    let y: u8 = 0b1100; // 12

    println!("x & y = {:b}", x & y);
    println!("x | y = {:b}", x | y);
    println!("x ^ y = {:b}", x ^ y);
    println!("!x = {:b}", !x);
    println!("x << 1 = {:b}", x << 1);
    println!("x >> 1 = {:b}", x >> 1);
}
```

输出：

```bash
x & y = 1000
x | y = 1110
x ^ y = 110
!x = 11110101
x << 1 = 10100
x >> 1 = 101
```

### 5、赋值与复合赋值运算符

| 运算符 | 说明 | 示例 | 结果 |
|---|---|---|---|
| = | 赋值 | let mut x = 5; x = 3; | x = 3
| += | 加并赋值 | x += 2 | x = x + 2
| -= | 减并赋值 | x -= 2 | x = x - 2
| *= | 乘并赋值 | x *= 2 | x = x * 2
| /= | 除并赋值 | x /= 2 | x = x / 2
| %= | 取余并赋值 | x %= 2 | x = x % 2
| &= |= ^= <<= >>= | 位运算复合赋值 | x &= 2 | 类似

实例

```rust
fn main() {
    let mut n = 5;

    n += 3;
    println!("n += 3 -> {}", n);

    n *= 2;
    println!("n *= 2 -> {}", n);

    n >>= 1;
    println!("n >>= 1 -> {}", n);
}
```

输出：

```bash
n += 3 -> 8
n *= 2 -> 16
n >>= 1 -> 8
```

### 6、 其他常见运算符

| 运算符 | 说明 | 示例
|---|---|---|
| .. | 范围（不含右端） | 0..5 产生 0 到 4
| ..= | 范围（含右端） | 0..=5 产生 0 到 5
| as | 类型转换 | 5 as f32
| ? | 错误传播（在 Result 中） | some()?;
| * | 解引用 | *ptr
| & | 取引用 | &x
| ref | 绑定为引用 | let ref y = x;

实例

```rust
fn main() {
    let x = 5;
    let y = x as f64;

    for i in 1..4 {
        print!("{} ", i);
    }
    println!();

    for i in 1..=3 {
        print!("{} ", i);
    }
    println!();

    let a = 10;
    let b = &a;
    println!("*b = {}", *b);
}
```

输出：

```bash
1 2 3 
1 2 3 
*b = 10
```