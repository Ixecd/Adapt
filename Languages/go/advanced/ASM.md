## ASM 汇编
基于 Plan9 汇编风格
保存在 `.s` 文件中，编译器自动编译、链接

采用 `stack-based` 传参模式, 1.17之后使用 寄存器 传参

**注意本文都是以AMD64架构为例**

```Plan9
Intel					AT & T 					Go
mov eax, 1				movl $1, %eax			MOVQ $1, AX
mov rbx, 0ffh			movl $0xff, %rbx		MOVQ $(0xff), BX
mov ecx, [ebx + 4]		movl 3(%rbx), %ecx		MOVQ2(BX), CX
```
指令参数长度
- MOVB: 1 byte
- MOVW: 2 bytes
- MOVL: 4 bytes
- MOVQ: 8 bytes

数据移动方向: 从左往右

```asm
ADD R1, R2			// R2 += R1
SUB R3, R4			// R4 -= R3
SUB R3, R4, R5		// R5 = R4 - R3
MUL $7, R6			// R6 *= 7
```
内存访问
```asm
MOV (R1), R2			// R2 = *R1
MOV 8(R1), R2			// R2 = *(R1 + 8)
MOV 16(R1)(R2<<1), R3	// R3 = *(R1 + 16 + R2*2)
MOV runtime·x(SB), R2	// R2 = *runtime·x
```
跳转指令
```asm
JMP label			// 跳转到 label 标签处
JMP 2(PC)			// 跳转到 PC + 2 处
JMP -2(PC)			// 跳转到 PC - 2 处
```
数字常量以 `$` 开头。十进制`$10`，十六进制`$0x10`

编译器引入`FUNCDATA`和`PADATA`，包含垃圾回收信息

## 寄存器
伪寄存器由汇编语言定义并使用，最终会被编译器替换为硬件寄存器

```bash
# 输出结果已经没有伪寄存器
go build -gcflags -S

# 结果也没有伪寄存器，更干净
go tool objdump
```

- **SB: Static Base Pointer（静态基址指针）**

表示内存起始位置，通常用于全局函数或数据。例如: `CALL add(SB)` 表示函数 `add` 的地址

添加尖括号`add<>(SB)`，表示仅当前文件内可见，私有成员。还可以添加偏移量，表示基于符号的地址，例如: `add+8(SB)`

- **FP: Frame Pointer（帧指针）**

指向参数列表起始地址，以偏移量指向不同参数或返回值。偏移量前包含参数名:`symbol+offset(FP)`
如果没有参数名，无法编译。例如: `size+16(FP)`表示 `size = (FP + 16)`

- **SP: Stack Pointer（栈指针）**

当前栈帧内，指向本地局部变量起始地址。例如: `size+16(SP)`表示 `size = (SP + 16)`

- **PC: Program Counter（程序计数器）**

按指令行数跳转。例如`JMP 2(PC)`表示以当前位置为0基准，往下跳2行

## 段
不同于 NASM 使用的section定义，直接在符号前添加类别
- `DATA`: 初始化全局符号内存
- `GLOBL`: 声明符号是全局的

## 函数
函数定义，指定栈帧和参数大小
```asm
TEXT main·add(SB), NOSPLIT, $8-24
```
上面main是包名，add是函数名，8是局部栈帧大小（不包含参数、返回值，以及环境保存），24是参数以及返回值大小