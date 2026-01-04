## Makefile核心语法

### Makefile规则语法

Makefile的规则语法，主要包括target、prerequisites和command

```mk
target ...: prerequisites ...
	command
		...
		...
```
- target，可以是一个object file（目标文件），也可以是一个执行文件，还可以是一个标签（label）。target可使用通配符，当有多个目标时，目标之间用空格分隔。
- pererquisites，代表生成该target所需要的所有依赖项。当有多个依赖项时，依赖项之间用空格分隔。
- command，代表该target要执行的命令（可以是任意的shell命令）
	1. 在执行command之前，默认会将该命令打印，然后输入命令的结果，如果不想打印出命令，可以在各个command前面加上@
	2. command可以为多条，可以分行写，每行都要以tab键开始。如果后一条命令依赖前一条命令，则这两条命令需要写在同一行，并用分号进行分隔。
	3. 如果要忽略命令的出错，需要在各个command之前加上减号 - 。

**只要target不存在，或者pererquisites中有一个以上的文件比target文件新，那么command所定义的命令就会被执行，从而产生我们需要的文件，或执行我们期望的操作**

### 伪目标

伪目标，我们不会为该目标生成任何文件。因为伪目标不是文件，make无法生成它的依赖关系，也无法决定是否要执行它。

通常情况下，需要显式地识别这个目标为伪目标，在Makefile中可以使用`.PHONY`来标识一个目标为伪目标
```mk
.PHONY: clean
clean:
	rm hello.o
```
伪目标可以有依赖文件，也可以作为「默认目标」，例如：
```mk
.PHONY: all
all: lint test build
```
因为伪目标总是会被执行，所以其依赖总是会被决议，通过这种方式，可以达到同时执行所有依赖项的目的
**order-only依赖**
有时候，我们希望只有当prerequisites中的部分文件改变时，才重新构造target
```mk
targets : normal-pre | order-only-pre
	command
	...
	...
```
上面的规则中，只有第一次构造target时，才会使用order-only-pre。后面即使order-only-pre发生变化，也不会重新构造target，只有normal-pre中的文件发生变化时，才会重新构造target，这里的 `|` 后面的pre就是order-only-pre

### Makefile语法概览
**命令**

Makefile支持Linux命令，调用方式跟在Linux系统下调用命令的方式一致。默认情况下，make会把正在执行的命令输出到当前屏幕上。可以通过添加`@`符号的方式，禁止make输出当前正在执行的命令

默认情况下，每条命令执行完make就会检查其返回码。如果返回成功（返回码为0），make就执行下一条指令；如果返回失败（返回码非0），make就会终止当前命令。很多时候，命令出错（比如删除了一个不存在文件）时，我们并不想终止，可以在命令行前面加一个 `-` 符号，来让make忽略命令的出错，以继续执行下一条命令，比如：
```mk
clean:
	-rm hello
```

**变量**

Makefile支持变量赋值、多行变量和环境变量，Makefile内置了一些特殊变量和自动化变量

Makefile在使用变量时，会像shell变量一样原地展开，然后再执行替换后的内容

Makefile可以通过变量声明来声明一个变量，变量在声明时需要赋予一个初始值

引用变量的方式可以是`${}`或者`$()`。这里推荐使用`$()`，也建议整个mk文件的变量引用方式保持一致

变量会想bash变量一样，在使用的地方展开
```mk
GO=go
build:
	$(GO) build -v .
```
**变量赋值**
1. `=` 最基本的赋值方式
	- 在用变量给变量赋值时，右边变量的取值，取的是最终的变量值
2. `:=` 直接赋值，赋予当前变量的值
3. `?=` 表示如果该变量没有被赋值，则赋予等号后面的值
4. `+=` 表示将等号后面的值添加到前面的变量上

**多行变量**
```mk
define 变量名
变量内容
...
endef
```
变量的内容可以包含函数、命令、文字或者其他变量

**环境变量**
在Makefile中，有两种环境变量，分别是Makefile预定义的环境变量和自定义的环境变量

自定义的环境变量可以覆盖Makefile预定义的环境变量

默认情况下，Makefile中定义的环境那边来那个只在当前Makefile中有效，如果向下层传递(Makefile中调用另外一个Makefile),需要使用export关键字来声明
```mk
...
export USAGE_OPTIONS
...
```

**特殊变量**

特殊变量是make提前定义好的，可以在makefile中直接引用

```txt
变量							含义
MAKE					当前make解释器的文件名
MAKECMDGOALS 			命令行中指定的目标明
CURDIR					当前make解释器的工作目录
MAKE_VERSION			当前make解释器的版本
MAKEFILE_LIST			make所需要处理的makefile文件列表
						当前makefile的文件名总是位于列表的最后
						文件名之间以空格进行分隔
.DEFAULT_GOAL			指定如果在命令行中未指定目标，应该构建
						哪个目标，即使这个目标不是在第一行
.VARIABLES				所有已经定义的变量名列表
.FEATURES				列出本版本支持的功能，以空格隔开
.INCLUDE_DIRS			make查询makefile的路径，以空格隔开
```
**自动化变量**

所谓自动化变量，就是这种变量会把模式中所定义的一些列的文件自动地挨个取出，一直到所有符合模式的文件都取完为止。这种自动化变量应该只出现咋及规则的命名中。Makefile中支持的自动化变量见下表
```mk
变量							含义
$@						表示规则中的目标文件集，可以是集合
$%						仅当目标是函数库文件中，表示规则中的目标成员名
$<						依赖目标中的第一个目标名称，可以是集合
$?						所有比目标新的依赖目标集合，以空格分隔
$^						所有的依赖目标的集合，以空格分隔
$+						和$^作用一样，只是不去重
$|						所有的order-only依赖目标集合，以空格分隔
$*						目标模式中%及其前面的部分。如果目标是
						dir/a.foo.b，并且目标的模式是a.%.b那么
						$*的值就是dir/a.foo
```
上面的自动化变量中，使用最多的是`$*`。其对于构造有关联的文件名是非常有效果的。如果目标中没有模式的定义，那么`$*`就不能被推导。但是，如果目标文件的后缀名是make能够识别的，那么`$*`就是除了后缀的那一部分

### 条件语句
下面是一个栗子
```mk
ifeq ($(ROOT_PACKAGE),)
$(error the variable ROOT_PACKAGE is not set)
else
$(info the value of ROOT_PACKAGE is $(ROOT_PACKAGE))
endif
```
1. ifeq 条件判断，判断是否相等
2. ifneq 条件判断，判断是否不相等
3. ifdef 条件判断，判断变量是否已定义
4. ifndef 条件判断，判断变量是否未定义

**函数**

Makefile同样也支持函数，函数语法包括定义语法和调用语法

**自定义函数**
```mk
define 函数名
函数体
endef
```
下面是一个栗子
```mk
define Foo
	@echo "my name is $(0)"
	@echo "param is $(1)"
endef
```
define 本质上是定义一个多行变量，可以在call的作用下当做函数来使用，在其他位置只能作为多行变量来使用
```mk
var := $(call Foo)
new := $(Foo)
```

**预定义语法**
make编译器也定义了很多函数，这些函数叫作预定义函数，调用语法和变量类似，语法为
```mk
$(<function> <arguments>)
```
或者
```mk
${<function> <argements>}
```
参数之间使用逗号分割
下面是一个栗子
```mk
PLATFORM = linux_amd64
GOOS := $(word 1, $(subst _, ,$(PLATFORM)))
```
Makefile预定义函数包括以下这些
```text
	函数名									功能描述
$(origin <variable>)				告诉变量的「出生情况」，有如下返回值
									undefined: <variable>从来没有定义过
									default: <variable>是一个默认的定义
									environment: <variable>是一个环境变量
									file: <variable>这个变量被定义在Makefile中
									command line: <variable>这个变量是被命令行定义的
									override: <variable>是被override指示符重新定义的
									automatic: <variable>是一个命令运行中的自动化变量
$(addsuffix <suffix>,<names...>)	把后缀<suffix>加到<names>中的每个单词的后面
$(addprefix <prefix>,<names...>)	把前缀<prefix>加到<names>中的每个单词的前面
$(wildcard <pattern>)				扩展通配符，例如$(wildcard $(ROOT_DIR)/build/docker/*)
$(word <n>,<text>)					取字符串<text>中第<n>个单词，并返回字符串<text>中第<n>个单词
									如果<n>比<text>中的单词大，那么返回空字符串
$(subst <from>,<to>,<text>)			把字符串<text>中的<from>字符串替换为<to>，并返回替换后的字符串
$(eval <text>)						将<text>作为makefile的一部分而被make解析和执行
$(abspath <text>)					绝对路径
$(filter <pattern...>,<text>)		以<pattern>模式过滤掉<text>字符串中的单词，保留符合<pattern>的单词
$(filter-out <pattern...>,<text>)	去除符合<pattern>的单词
$(foreach <var>,<list>,<text>)		把参数<list>中的单词逐一取出，放到参数<var>所指定的变量中，
									然后执行<text>，返回结果以空格分隔组成整个字符串
```
### 引入其他Makefile
通过关键字include，把别的makefile包含进来，被包含的文件会插入到当前位置
```mk
include scripts/make-rules/common.mk
include scripts/make-rules/golang.mk
```
可以包含通配符