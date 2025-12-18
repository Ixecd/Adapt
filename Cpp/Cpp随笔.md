# routine_test
1. Cpp标准库中begin和end一般用于成员函数,first和last用于函数参数

2. 关于 int 和 size_t(uintptr_t)
    在比较中如果同时出现这两个,编译器默认会将int转换为size_t,这在判断 >=0 ? --i 非常危险!

3. 关于VsCode中 F5
    F5 对应 tasks.json 和 c_cpp_properties.json 和 launch.json, 三个文件里编译器的版本都要正确F5才能正确RUN, 前两个负责编译,最后一个负责运行

4. 关于模板
    除了一般常见的模板C++20提供了#include<iterator>, template <std::random_access_iterator InputIt> -> 约束类型为随机访问迭代器(约束机制) -> 使用概念(concepts)约束类型参数
    std::random_access_iterator是C++20引入的一个概念(concept)
    一下是一些常见的概念:
        std::integral:用于整数类型。
        std::floating_point:用于浮点类型。
        std::signed_integral:用于带符号整数类型。
        std::unsigned_integral:用于无符号整数类型。
        std::input_iterator:用于输入迭代器。
        std::output_iterator:用于输出迭代器。
        std::forward_iterator:用于前向迭代器。
        std::bidirectional_iterator:用于双向迭代器。
        std::random_access_iterator:用于随机访问迭代器。
    模板分为类型模板和非类型模板
    template <class T> 和 template <size_t N> 后者用于传递常量值

5. 关于STL中的容器
    vector:在vector中迭代器直接就是T*,类中成员只有T* m_datas, size_t m_size, size_t capacity.扩容的关键是std::max(n, (1.5)or(2.0)*m_capacity), n是当前想要的大小.在vector中分配器使用
        std::allocator<T>就行但要保证这个分配器是有状态的.如果直接使用new(malloc)有小概率得到的内存是不对其的.并且使用new得到的内存会被赋值一遍,使用分配器得到的内存是干净的,之后再到构造函数中使用
        std::constrcut_at来给成员变量赋值.之后要注意的就是各种构造函数/重载函数/操作函数,要注意各种const的使用(对数据的操作,迭代器的操作都要有两个版本).作为函数参数或者返回值时, T const & equal
        to T,推荐使用前者,会少一次拷贝构造函数,拷贝构造函数的类型也为&最好是const &,因为在C++的实现中,const &可以传递右值.还要注意insert中元素的搬移方向.使用C++20中template<concepts Input>
        来约束函数类型.
    list:再list中迭代器需要单独编写,并且有两个版本itearto和const_iterator,这里的const是常量指针,表示的是指针指向的内容不能改,仍然可以进行operat++/--操作.list中目前的成员只有struct ListNode, iterator,
        const_iterator, 和 ListNode dummy;list是双向循环链表,dummy表示end,之后就是各种构造函数/重载函数(尤其是对迭代器++/++(int)/--/--(int)/*/==/!=),还有各种对迭代器的操作(const / non_const...)

6. 关于iterator
    input_iterator 支持 *it it++ ++it it!=it it==it
    output_iterator 支持 *it=val it++ ++it it!=it it==it
    forward_iterator 支持 *it *it=val it++ ++it it!=it it==it
    bidirectional 支持 *it *it=val it++ ++it --it it-- it!=it it==it
    random_access_iterator 支持 *it *it=val it[n] it[n]=val it++ ++it it+=n --it it-- it-=n it+n it-n it!=it it==it
    下面的是上面的超集
    一般list、set、map的iterator类型为bidirectional,如果想要和random_access_iterator效果一样的话需要使用std::advance(it, n) n > 0向后移动 n < 0 向前移动.

7. 关于lambda
    在类模板中可以使用lambda作为类型,用户调用的时候直接在参数中写自己的lambda函数,在函数体中直接就会调用用户写的这个函数,小技巧.也可以直接使用template<std::invocable<T const &>Visitor> 更方便(俩版本).

8. 关于noexcept
    能加的都加上,编译器很多时候不对代码进行优化的原因就是害怕程序抛出异常,加上之后允许编译器尽情优化.

9. 关于[[no_unique_address]] 和 union
    union 允许多个成员变量共享同一块内存空间,每个成员变量之间是互斥的,任何时刻只有一个成员有效.
    [[no_unique_address]] 主要是用在类中有空类成员变量的情况,比如分配器,理论上来说空类并不需要内存,但是编译器默认会分配1个byte,由于内存对齐会导致较多浪费,所以使用[[no_unqiue_address]]让编译器优化内存布局,减少类的大小.

10. 关于减少类大小的方法
    针对空类可以使用[[no_unique_address]]修饰
    针对类中有双向链表,使用继承将value封装到子类中,并使用union修饰, 因为数据成员在类的实例化和使用时才会占用内存，而不是在定义类时立即占用内存

11. 关于list中成员变量的设计
    通过使用"哑节点",也就是指针和数据分离的方式,可以很好的将val隐藏起来,切记使用union将val包含,这样只有在类实例化和使用的时候才会占用内存.C++中union中的成员并不会自动调用构造函数

12. 关于const & 和 &&
    const &, 可以传递右值编译器会临时创建一个对象保存这个值, 和&&完全不同.const & 是为了避免拷贝同时保证不会修改, 作为参数时,&&会退化为 const&

13. 关于inline 和 static
    目前inline的作用已经逐渐脱离之前的原则,在函数前面加上inline在编译的时候不选优化O2..编译器就永远不会优化.
    为了解决在头文件中编写static变量、函数,从而多个cc文件include它就会生成多个副本,各自用各自的互不干扰,但是可执行文件会变大.
    目前其作用主要体现在hpp文件中,用于多单元共享变量使用,inline int i; 表示i可以打破ODR(one definition rule)可以有多个定义,如果定义的值不同,链接器就会选择随机的一个作为值,之后覆盖其它值,
    但本质上这是未定义行为,但不会报错,因为这是链接器的工作,所以在hpp文件中定义inline 这样无论是在main.cc中还是其他.cc中包含这个头文件只会有一份.会减小用static修饰的可执行文件大小

    在源文件中定义一次,在头文件中声明也可以解决问题.

    在头文件的class中成员函数的定义都默认是inline,而声明时extern
    如果在头文件中的class中的成员函数是声明,而在这个头文件中在类的外面定义了函数,这个时候是需要在类中的成员函数加上inline的,因为此时为extern

    对于不同编译单元需要不同函数体的同名函数,这时候就要使用static

    如果有多个编译单元但是有需求必须在这两个编译单元中定义同名类,这时候究极解决方案就是在这个类外面套一层匿名namespace

14. 关于git clone
    在git clone 的时候加上参数 --depth=1 会更快一点,表示只clone最近的一次commit,不加的话会把所有commit都clone

15. std::upper_bound 和 std::lower_bound
    仅支持随机访问迭代器random_access_iterator,因为内部实现是二分.
    

16. 优化等级(GCC)
- -O0:不进行任何优化,编译速度最快,忠实复刻你写的代码,未定义行为不容易产生诡异的结果
- -O1:最基本的优化,把简单的死代码删除
- -O2:会内联函数,减少函数调用,把一些简单的数组操作用更快的指令替代,执行速度更快
- -O3:比O2更激进的优化,把一些复杂的循环用SIMD矢量指令优化加速,把一些复杂的数组操作用更快的指令代替等,性能提升很大,如果程序犹未定义行为,可能会导致一些BUG,编译速度会很慢,一般用于程序最终发布阶段
- Ofast:在O3的基础上,进一步对浮点数的运算进行更深层次的优化,但是可能会导致一些浮点数计算结果不准确,如果成需不涉及到NaN和Inf的处理,那么使用它不会有太大问题
- Os:在O2的基础上,专门优化代码大小,性能被当作次要要求,但是会禁止会导致可执行文件变大的优化,会把一些循环展开,内联等优化关闭,把一些代码使用更小的指令实现,进程的减少可执行文件的大小
- Oz:在O使得基础上,进一步把代码压缩,为了缩少尺寸牺牲运行时性能
- Og:在O0的基础上,尽可能地保留更多的调试信息