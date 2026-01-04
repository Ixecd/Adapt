/**
 * @file qc_vector.cc
 * @author qc
 * @brief 扩充vector类
 * @details review
 * @version 1.1
 * @date 2024-07-07
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <thread>
#include <utility>
/**
 * @brief int是一个POD类型,可以平凡拷贝,所以才可以memcpy,
 * 如果是string类型,里面有自定义的拷贝构造函数,就不能使用memcpy
 */

namespace qc {

// struct S {
//     qc::vector<int> v;
//     std::allocator<int> a;
// };

template <class T, class Alloc = std::allocator<T>>
/**
 * @brief int是一个POD类型,可以平凡拷贝,所以才可以memcpy,
 * 如果是string类型,里面有自定义的拷贝构造函数,就不能使用memcpy
 */

class vector {
public:
    // 分配器 allocator 一般作为一个成员,为了支持实现内存池,alloc是有状态的
    // 普通allocator是调用全局的new和delete,无状态 空基类优化peace
    // 分配得到的内存一定是对其的,裸malloc可能得到的内存不对其
    using allocator = std::allocator<T>;
    using value_type = Alloc;
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T const&;
    using difference_type = ptrdiff_t;
    using size_type = size_t;
    using iterator = T*;
    using const_iterator = T const*;
    using reverse_iterator = std::reverse_iterator<T*>;
    using const_reverse_iterator = std::reverse_iterator<T const*>;

public:
    // 杜绝隐式转换,避免用户手滑写错的情况 vector arr = 3 ==> want ->arr {3},
    // don't want -> arr {0, 0, 0} explicit vector(size_t n) {
    //     m_data = new T[n]{};
    //     memset(m_data, 0, n * sizeof(T));
    //     m_size = n;
    //     m_capacity = n;
    // }
    vector(Alloc const& alloc = allocator()) noexcept : m_alloc(alloc) {
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    explicit vector(size_t n, allocator const& alloc = Alloc())
        : m_alloc(alloc) {
        // m_data = new T[n]{val}; 只会初始化第一个为val
        // m_data = new T[n];
        // m_data = allocator{}.allocate(n);
        m_data = m_alloc.allocate(n);
        m_size = m_capacity = n;
        for (size_t i = 0; i < m_size; ++i) std::construct_at(&m_data[i]);
    }
    // 常量引用可以绑定右值->编译器临时创建一个对象保存这个右值(通常在栈上)
    vector(size_t n, T const& val, allocator const& alloc = allocator())
        : m_alloc(alloc) {
        // m_data = new T[n]{val}; 只会初始化第一个为val
        // m_data = new T[n];
        // m_data = allocator{}.allocate(n);
        m_data = m_alloc.allocate(n);
        m_size = m_capacity = n;
        for (size_t i = 0; i < m_size; ++i)
            std::construct_at(&m_data[i], std::move(val));
    }

    // 这里允许隐式转换,一般类型直接推导即可
    // c20标准 用来约束模板InputIt必须是 random_access_iterator(概念
    // concept)类型
    template <std::random_access_iterator InputIt>
    vector(InputIt first, InputIt last, allocator const& alloc = allocator())
        : m_alloc(alloc) {
        size_t n = last - first;
        // m_data = new T[n];
        // m_data = allocator{}.allocate(n);
        m_data = m_alloc.allocate(n);
        for (size_t i = 0; i < n; ++i) std::construct_at(&m_data[i], *first++);
        m_capacity = m_size = n;
    }

    vector(std::initializer_list<T> ilist, allocator const& alloc = allocator())
        : vector(ilist.begin(), ilist.end(), alloc) {}

    // 拷贝构造函数, 深拷贝
    vector(const vector& v, allocator const& alloc = allocator())
        : m_alloc(alloc) {
        m_capacity = m_size = v.m_size;
        if (m_size) {
            // 不用new了,避免一轮赋值, malloc出来的不一定是对其的
            // m_data = malloc(m_size * sizeof(T));
            // m_data = allocator{}.allocate(m_capacity);
            // m_data = new T[m_size]; //
            m_data = m_alloc.allocate(m_size);
            // 如果是string类型即使不加{}也会初始化的时候进行一轮赋值,下面还会进行一轮赋值
            for (size_t i = 0; i < m_size; ++i) {
                // m_data[i] = std::as_const(v.m_data[i]);  // ->
                // T类型的拷贝赋值函数,推荐走const 版本
                std::construct_at(&m_data[i], std::as_const(v.m_data[i]));
                // or
                // new (&m_data[i]) T (std::as_const(that.m_data[i]));
            }
            // memcpy(m_data, v.m_data, sizeof(T) * m_size);
        } else {
            m_data = nullptr;
        }
    }

    // 移动构造函数 把v的变成自己的
    // 不需要深拷贝
    // 移动构造一般都用noexcept修饰,不会申请新的内存
    vector(vector&& v) noexcept : m_alloc(std::move(v.m_alloc)){
        // std::cout << "in vector(vector&& v) " << std::endl;
        m_data = v.m_data;
        m_size = v.m_size;
        m_capacity = v.m_capacity;
        v.m_data = nullptr;
        v.m_size = 0;
        v.m_capacity = 0;
    }

    // 移动赋值函数
    vector& operator=(vector&& v) noexcept {
        // std::cout << "in operator=(vector&& v) " << std::endl;
        // clear();
        if (m_size) [[likely]] 
            for (size_t i = 0; i < m_size; ++i) 
                std::destroy_at(&m_data[i]);
        if (m_capacity) [[likely]]
            m_alloc.deallocate(m_data, m_capacity);
        // delete[] m_data;
        m_data = v.m_data;
        m_size = v.m_size;
        m_capacity = v.m_capacity;
        v.m_data = nullptr;
        v.m_size = 0;
        v.m_capacity = 0;
        return *this;
    }

    // 拷贝赋值函数
    vector& operator=(vector const& v) {
        // 判断是否自我赋值,判断的是地址
        if (&v == this) return *this;
        // clear();
        reserve(v.m_size);
        m_capacity = m_size = v.m_size;
        if (m_size) {
            // m_data = new T[m_size]{};
            for (size_t i = 0; i < m_size; ++i)
                // m_data[i] = std::as_const(v.m_data[i]);
                std::construct_at(&m_data[i], std::as_const(v.m_data[i]));
            // memcpy(m_data, v.m_data, sizeof(T) * m_size);
        }
        return *this;
    }

    Alloc get_allocator() const noexcept { return m_alloc; }

    bool operator==(vector const& that) noexcept {
        return std::equal(begin(), end(), that.begin(), that.end());
    }

    // operator = 中无法实现多个参数
    // assign相当于祛魅版
    template <std::random_access_iterator InputIt>
    void assgin(InputIt first, InputIt last) {
        size_t diff = last - first;
        reserve(diff);
        for (size_t i = 0; i < diff; ++i) {
            std::construct_at(&m_data[i], std::as_const(*first++));
        }
    }

    void assgin(size_t n, T const& val) {
        clear();
        reserve(n);  // reserve -> 容量扩增并不会改变m_size
        m_size = n;
        for (size_t i = 0; i < n; ++i) {
            std::construct_at(&m_data[i], val);
        }
    }
    // initializer_list直接转发给迭代器版本即可
    void assign(std::initializer_list<T> ilist) noexcept {
        assign(ilist.begin(), ilist.end());
    }

    ~vector() noexcept {
        for (size_t i = 0; i < m_size; ++i) 
            std::destroy_at(&m_data[i]);
        if (m_capacity) [[likely]]
            m_alloc.deallocate(m_data, m_capacity);
    }

public:
    void clear() noexcept {
        for (size_t i = 0; i < m_size; ++i) std::destroy_at(&m_data[i]);
        m_size = 0;
    }
    // 赋值函数一定要把之前的释放掉
    // 要把原来的东西拷贝过去
    // _name 内部api
    void _recapacity(size_t n) {
        {
            if (m_capacity) {
                if (0 == n) {
                    // delete[] m_data;
                    allocator{}.deallocate(m_data, m_capacity);
                    m_data = nullptr;
                    m_capacity = 0;
                    return;
                }
                // new出来之后使用{}初始化为0
                // T* tmp = new T[n]{};
                T* tmp = allocator{}.allocate(n);
                // 如果size变小就要把多余的砍掉
                for (size_t i = 0; i < std::min(m_capacity, n); ++i)
                    std::construct_at(&tmp[i], std::as_const(m_data[i]));
                // memcpy(tmp, m_data, std::min(m_capacity, n) * sizeof(T));
                // delete[] m_data;
                allocator{}.deallocate(m_data, m_capacity);
                m_data = tmp;
                m_capacity = n;
            } else {
                if (0 == n) return;
                // m_data = new T[n]{};
                m_data = allocator{}.allocate(n);
                m_capacity = n;
            }
        }

        // {
        //     T* old_data = m_data;
        //     size_t old_size = m_size;
        //     if (n == 0) {
        //         if (m_data) {
        //             delete[] m_data;
        //             m_data = nullptr;
        //             m_size = 0;
        //         }
        //         return;
        //     }
        //     m_data = new T[n]{};
        //     m_size = n;
        //     memcpy(m_data, old_data, std::min(old_size, n) * sizeof(T));
        //     if (old_data)
        //         delete[] old_data;
        // }
    }

    // 缩小容量(请求,并不一定会执行)
    // 不想浪费内存,将大内存释放掉,得到小内存
    void shrink_to_fit() {
        T* old_data = m_data;
        size_t old_capacity = m_capacity;
        m_capacity = m_size;
        // m_size == 0 不代表没有new过
        if (m_size == 0) {
            m_data = nullptr;
        } else {
            // m_data = allocator{}.allocate(m_size);
            // m_data = new T[m_size];
            m_data = m_alloc.allocate(m_size);
            for (size_t i = 0; i < m_size; ++i)
                std::construct_at(&m_data[i], std::as_const(old_data[i]));
            // memcpy(m_data, old_data, m_size * sizeof(T));
        }
        if (old_capacity) m_alloc.deallocate(old_data, old_capacity);
        // delete[] old_data;
    }
    // 扩容从0到1的本质是max(n, m_capacity * 2)
    // 如果一开始size = 0, 那push_back的时候这个函数的n就是1, 之后扩充
    void _grow_capacity_until(size_t n) {
        if (n <= m_capacity) return;
        // 两倍增长
        T* old_data = m_data;
        size_t old_capacity = m_capacity;
        n = std::max(n, m_capacity * 2);
        printf("vector capacity from %zd to %zd\n", m_capacity, n);
        if (n == 0) {
            m_data = nullptr;
            m_capacity = 0;
        } else {
            // m_data = new T[n]{};
            // m_data = allocator{}.allocate(n);
            m_data = m_alloc.allocate(n);
            m_capacity = n;
        }
        if (old_capacity) {
            if (m_size)
                for (size_t i = 0; i < m_size; ++i)
                    std::construct_at(
                        &m_data[i],
                        std::as_const(old_data[i]));  // std::move(old_data[i])
            // memcpy(m_data, old_data, sizeof(T) * m_size);
            // delete[] old_data;
            // allocator{}.deallocate(old_data, old_capacity);
            m_alloc.deallocate(old_data, old_capacity);
        }
    }

    void resize(size_t n) {
        _grow_capacity_until(n);
        if (n > m_size) {
            for (size_t i = m_size; i < n; ++i)
                std::construct_at(&m_data[i]);  // => m_data[i] = 0
            // m_data[i] = val;
        }
        m_size = n;
    }

    void resize(size_t n, T const& val) {
        _grow_capacity_until(n);
        if (n > m_size) {
            for (size_t i = m_size; i < n; ++i)
                std::construct_at(&m_data[i], std::move(val));
            // m_data[i] = val;
        }
        m_size = n;
    }

    void reserve(size_t n) {
        // 不考虑之前有元素
        // T *old_data = m_data;
        // m_capacity = n;
        // m_data = new T[n]{};
        // m_size = 0;
        // delete[] old_data;
        _grow_capacity_until(n);
    }

public:
    /// @brief 下标类型一定是size_t类型,如果是int最多表示2亿也就是2^31 - 1,
    /// size_t -> 2 ^ 64 - 1 arr[i] = i -->
    /// 如果返回int为纯右值,返回之后存到rax寄存器了,没啥效果
    T& operator[](size_t i) noexcept { return m_data[i]; }

    // 这里如果直接返回T会产生一次拷贝,开销可能会很大
    const T& operator[](size_t i) const noexcept { return m_data[i]; }

    // 提供越界保护
    T& at(size_t i) {
        // 小概率事件 加[[unlikely]] 修饰, 编译器能优化的好一点,初始化
        if (i >= m_size) [[unlikely]]
            throw std::out_of_range("vector::at");
        return m_data[i];
    }

    T const& at(size_t i) const {
        if (i >= m_size) [[unlikely]]
            throw std::out_of_range("vector::at");
        return m_data[i];
    }

    T& back() noexcept { return m_data[m_size - 1]; }

    T const& back() const noexcept { return m_data[m_size - 1]; }

    T& front() noexcept { return m_data[0]; }

    T const& front() const noexcept { return m_data[0]; }

    size_t size() const noexcept { return m_size; }

    bool empty() const noexcept { return m_size == 0; }

    size_t capacity() const noexcept { return m_capacity; }

public:
    // 迭代器, 注意在Class中 typedef也受访问控制
    // typedef T* iterator;

    const iterator begin() const noexcept { return m_data; }

    const iterator end() const noexcept { return m_data + m_size; }

    const iterator cbegin() const noexcept { return m_data; }

    const iterator cend() const noexcept { return m_data + m_size; }

    std::reverse_iterator<iterator> rbegin() noexcept {
        return std::make_reverse_iterator(m_data);
    }

    std::reverse_iterator<iterator> rend() noexcept {
        return std::make_reverse_iterator(m_data + m_size);
    }

    std::reverse_iterator<const iterator> rcbegin() const noexcept {
        return std::make_reverse_iterator(m_data);
    }

    std::reverse_iterator<const iterator> rcend() const noexcept {
        return std::make_reverse_iterator(m_data + m_size);
    }

    iterator begin() noexcept { return m_data; }

    iterator end() noexcept { return m_data + m_size; }

    iterator data() noexcept { return m_data; }

    const iterator data() const noexcept { return m_data; }

    const iterator cdata() const noexcept { return m_data; }

public:
    void push_back(T const& val) {
        reserve(size() + 1);  // reserve得到的内存是纯的
        std::construct_at(&m_data[m_size], std::as_const(val));
        m_size += 1;
        // (*this)[size() - 1] = val;
    }

    void push_back(T&& val) {
        reserve(size() + 1);
        std::construct_at(&m_data[m_size], std::move(val));
        m_size += 1;
        // (*this)[size() - 1] = val;
    }

    template <class... Args>
    T& emplace_back(Args&&... args) {
        reserve(size() + 1);
        T* p = &m_data[m_size];
        std::construct_at(p, std::forward<Args>(args)...);
        m_size += 1;
        return *p;
        // (*this)[size() - 1] = val;
    }
    // 区间删除(下标版本) vector erase 不会造成内存重新分配,所以迭代器依然有效
    T* erase(size_t index) noexcept {
        for (size_t j = index + 1; j < m_size; ++j) {
            // 后面那个东西已经不需要移动给当前位置
            m_data[j - 1] = std::move(m_data[j]);
        }
        resize(size() - 1);  // -> size -= 1 刚好指向的就是要destroy的
        // 按道理来说空出来的要处理
        std::destroy_at(&m_data[m_size]);
        // size_t right = m_size - index - 1;
        // memcpy(m_data + index, m_data + index + 1, sizeof(T) * right);
        return const_cast<T*>(&m_data[index]);
    }

    T* erase(size_t ibeg, size_t iend) noexcept {
        size_t diff = iend - ibeg;
        for (size_t j = iend; j < m_size; ++j) {
            m_data[j - diff] = std::move(m_data[j]);
        }
        // 既然是减小size就不需要resize,resize里面不增大时仅仅是对m_size进行减小操作
        // resize(m_size - diff);
        m_size -= diff;
        for (size_t i = m_size; i < m_size + diff; ++i)
            std::destroy_at(&m_data[i]);

        return const_cast<T*>(&m_data[ibeg]);
    }

    // 区间删除(迭代器版本)
    T* erase(iterator const it) noexcept(std::is_nothrow_move_assignable_v<T>) {
        // // 这里最好不要使用迭代器来移动数据,但是用的话也没问题:)
        // 失效只是一时的,由于vector支持随机访问,所以迭代器并不需要每个都准确的清楚,只需要知道head就行
        // for (iterator i = it + 1; i < end(); ++i)
        //     *(i - 1) = std::move_if_noexcept(*i);
        size_t s = it - m_data;
        for (size_t i = s + 1; i != m_size; ++i)
            m_data[i - 1] = std::move_if_noexcept(m_data[i]);
        // resize(size() - 1); // 这里已经对m_size - 1
        m_size -= 1;
        std::destroy_at(&m_data[m_size]);  // 刚刚好
        return const_cast<T*>(it + 1);     // &m_data[s];
    }

    T* erase(iterator const first, iterator const last) noexcept {
        size_t start = first - m_data;
        size_t end = last - m_data;
        return erase(start, end);
        // size_t diff = last - first;
        // for (iterator it = last; it != end(); ++it) {
        //     *(it - diff) = std::move(*it);
        // }
        // resize(size() - diff);
        // return const_cast<T*>(last); //
        // 这时候last指向的就是删除玩后的第一个元素
    }

    T* insert(iterator const it, T&& val) {
        size_t j = it - m_data;
        // size_t l = end() - it;
        reserve(size() + 1);
        m_size += 1;

        // 像这种要将元素往后移动的情况必须倒过来
        // 如果添加的数量小于头尾之间的数量,就会造成覆盖情况
        for (int i = j; i >= (int)j; --i) {
            std::construct_at(&m_data[i + 1], std::move(m_data[i]));
            std::destroy_at(&m_data[i]);
        }
        // m_data[i + n] = std::move(m_data[i]);

        for (size_t i = j; i < j + 1; ++i)
            std::construct_at(&m_data[i], std::move(val));
        // m_data[i] = val;

        return const_cast<T*>(it);
    }

    T* insert(iterator const it, T const& val) {
        size_t j = it - m_data;
        // size_t l = end() - it;
        reserve(size() + 1);
        m_size += 1;

        // 像这种要将元素往后移动的情况必须倒过来
        // 如果添加的数量小于头尾之间的数量,就会造成覆盖情况
        for (int i = j; i >= (int)j; --i) {
            std::construct_at(&m_data[i + 1], std::move(m_data[i]));
            std::destroy_at(&m_data[i]);
        }
        // m_data[i + n] = std::move(m_data[i]);

        for (size_t i = j; i < j + 1; ++i) std::construct_at(&m_data[i], val);
        // m_data[i] = val;
        return const_cast<T*>(it);
    }

    T* insert(iterator const it, size_t n, T const& val) {
        size_t j = it - m_data;
        // size_t l = end() - it;
        if (n == 0) [[unlikely]]
            return const_cast<T*>(it);
        reserve(size() + n);
        m_size += n;

        // 像这种要将元素往后移动的情况必须倒过来
        // 如果添加的数量小于头尾之间的数量,就会造成覆盖情况
        for (int i = j + n - 1; i >= (int)j; --i) {
            std::construct_at(&m_data[i + 1], std::move(m_data[i]));
            std::destroy_at(&m_data[i]);
        }
        // m_data[i + n] = std::move(m_data[i]);

        for (size_t i = j; i < j + n; ++i) std::construct_at(&m_data[i], val);
        // m_data[i] = val;
        return const_cast<T*>(it);
    }

    T* insert(iterator const it, size_t n) {
        size_t j = it - m_data;
        size_t l = end() - it;
        if (n == 0) [[unlikely]]
            return it;
        reserve(size() + n);
        m_size += n;

        // 像这种要将元素往后移动的情况必须倒过来
        // 如果添加的数量小于头尾之间的数量,就会造成覆盖情况
        for (int i = j + n - 1; i >= (int)j; --i) {
            std::construct_at(&m_data[i + 1], std::move(m_data[i]));
            std::destroy_at(&m_data[i]);
        }
        // m_data[i + n] = std::move(m_data[i]);

        for (size_t i = j; i < j + n; ++i) std::construct_at(&m_data[i]);
        // m_data[i] = val;
        return const_cast<T*>(it);  // 去掉const
    }

    // C++20
    template <std::random_access_iterator InputIt>
    T* insert(iterator const it, InputIt first, InputIt last) {
        size_t n = last - first;
        size_t j = it - m_data;
        size_t l = end() - it;
        if (n == 0) [[unlikely]]
            return it;
        reserve(size() + n);
        m_size += n;
        // j ~ m_size -> j + n ~ m_size + n
        // 有BUG如果n小于他们之间的间隙就会出现覆盖丢失数据
        // for (size_t i = j; i < size(); ++i) {
        //     m_data[i + n] = std::move(m_data[i]);
        // }

        // 必须倒过来
        // std::cout << "l = " << l << std::endl;
        // 注意int 和 size_t类型的比较问题,如果比较时同时出现int 和 size_t
        // 按照size_t来处理

        // 数据从前往后搬必须从前面的尾开始
        // 数据从后往前搬必须从后面的头开始
        for (int i = j + l - 1; i >= (int)j; --i) {
            std::construct_at(&m_data[i + 1], std::move(m_data[i]));
            std::destroy_at(&m_data[i]);
            // m_data[i + n] = std::move(m_data[i]);
        }
        for (size_t i = j; i < j + n; ++i) {
            // m_data[i] = *first++;  // 先解引用再++
            std::construct_at(&m_data[i], std::move(*first++));
        }
        return const_cast<T*>(it);
    }

    T* insert(iterator const it, std::initializer_list<T> ilist) {
        return insert(it, ilist.begin(), ilist.end());
    }

private:
    T* m_data;
    size_t m_size;
    size_t m_capacity;
    // Alloc m_alloc
    // 如果直接写在这里会占用一个byte的空间,由于内存对齐,会浪费很多字节 使用
    // [[no_unique_address]] 表示不在乎这个变量的地址
    [[no_unique_address]] allocator m_alloc;
};

}  // namespace qc

template <class T>
void print(const std::string name, qc::vector<T>& ary) {
    std::cout << "print " << name << std::endl;
    for (auto it = ary.begin(); it != ary.end(); ++it) std::cout << *it << " ";
    std::cout << std::endl;
}

template <class T>
void print_size(const std::string name, qc::vector<T>& ary) {
    std::cout << name << " size() = " << ary.size() << std::endl;
}

// m_data = new std::string[m_size] -> 有默认构造函数,会自动初始化,m_size次赋值
// 之后再赋值一遍->拷贝赋值函数->m_size次赋值
// 两倍开销

void test_vector() {
    // const qc::vector<int> arr(3);
    // qc::vector<int> arr = 3; -> 隐式构造函数杜绝 -> {0, 0, 0}
    qc::vector<int> arr(4);

    qc::vector<int> ary;
    for (size_t i = 0; i < arr.size(); ++i) arr[i] = i + 1;

    // 一旦定义了析构函数必须同时定义拷贝构造函数/移动构造函数,否则double
    // free问题
    auto bar = arr;

    arr.resize(3);  // -> 覆盖了之前new的int[2],内存泄露

    for (size_t i = 0; i < bar.size(); ++i) {
        int val_1 = arr[i];          // -> non const -> once copy
        auto const& val_2 = arr[i];  // non const -> no copy
    }

    bar.resize(7);

    for (size_t i = 0; i < arr.size(); ++i) {
        printf("arr[%zd] = %d\n", i, arr[i]);
    }

    bar[3] = 100;

    for (size_t i = 0; i < bar.size(); ++i) {
        printf("bar[%zd] = %d\n", i, bar[i]);
    }

    std::cout << "==================================" << std::endl;

    qc::vector<int> a;
    a.reserve(4);
    for (size_t i = 0; i < 4; ++i) a.push_back(i);

    qc::vector<int> shrink;
    shrink.reserve(1000);
    shrink.shrink_to_fit();
    std::cout << "shrink.size() = " << shrink.size()
              << " shrink.capacity() = " << shrink.capacity() << std::endl;

    for (qc::vector<int>::iterator it = a.begin(); it != a.end(); ++it)
        printf("a = %d\n", *it);

    std::cout << "======== test_iterator ========" << std::endl;
    qc::vector<int> b;
    b.resize(10);
    int value = 1;
    for (qc::vector<int>::iterator it = b.begin(); it != b.end(); ++it) {
        *it = value++;
        std::cout << *it << std::endl;
    }
    qc::vector<int>::iterator it = b.begin() + 1;
    // 删除第二个元素
    b.erase(it);
    for (qc::vector<int>::iterator it = b.begin(); it != b.end(); ++it) {
        std::cout << *it << std::endl;
    }

    auto first = b.begin() + 1, last = b.begin() + 3;
    b.erase(first, last);
    for (qc::vector<int>::iterator it = b.begin(); it != b.end(); ++it) {
        std::cout << *it << std::endl;
    }

    std::cout << "========== test insert ============" << std::endl;

    qc::vector<int> c(5);
    for (int i = 0; i < c.size(); ++i) {
        c[i] = i + 1;
        std::cout << c[i] << "";
    }
    std::cout << std::endl;

    int c_1[5]{8, 7, 6, 5, 4};
    int *f_1 = c_1 + 1, *l_1 = c_1 + 3;
    c.insert(c.begin(), f_1, l_1);
    for (int i = 0; i < c.size(); ++i) std::cout << c[i] << " ";
    std::cout << std::endl;
    c.erase(c.begin());
    std::cout << "after erase" << std::endl;
    for (int i = 0; i < c.size(); ++i) std::cout << c[i] << " ";
    std::cout << std::endl;

    std::cout << "=========== assign ===========" << std::endl;
    qc::vector<int> d;
    d.assgin(5, 100);  // equal to -> d = qc::vector<int>(5, 100);
    for (auto it = d.begin(); it != d.end(); ++it) std::cout << *it << " ";
    std::cout << std::endl;

    std::cout << "=========== initializer_list ===========" << std::endl;
    d = qc::vector<int>({1, 2, 3, 4});
    for (auto it = d.begin(); it != d.end(); ++it) std::cout << *it << " ";
    std::cout << std::endl;

    qc::vector<int> e{1, 3, 4, 5};
    for (auto it = e.begin(); it != e.end(); ++it) std::cout << *it << " ";
    std::cout << std::endl;

    std::cout << "insert an initializer_list" << std::endl;
    e.insert(e.begin(), {1, 2, 3});
    std::cout << "after insert an initializer_list" << std::endl;
    for (auto it = e.begin(); it != e.end(); ++it) std::cout << *it << " ";
    std::cout << std::endl;
}

void test_move() {
    qc::vector<int> arr{1, 2, 3, 4};
    qc::vector<int> bar = std::move(arr);  // 移动构造函数
    qc::vector<int> ary{3, 2, 1};
    bar = std::move(ary);
    print<int>("bar", bar);

    print_size<int>("bar", bar);

    qc::vector<int> q(3);
    print<int>("q", q);
}

void test_copy_assign() {
    qc::vector<int> arr{1, 2, 4, 5, 6};
    qc::vector<int> brr{2, 3, 4, 6, 7};

    brr = brr;

    print<int>("arr", arr);
    print<int>("brr", brr);
}

void test_thread() {
    // qc::vector<std::thread> v1;
    // std::thread t1(test_copy_assign);
    // v1.push_back(std::move(t1));
}

void test_emplace_back() {
    struct S {
        int x;
        int y;
    };
    qc::vector<S> bar;
    bar.emplace_back() = {1, 2};  // 1, 2
    // bar.emplace_back(1).x = 1;
    // bar.emplace_back(1).y = 1; // (1) ->
    // 指向的位置的结构体的第一个值为1,返回的是引用.y->设置y的值

    std::cout << bar[0].x << " " << bar[0].y << std::endl;
}

// T* erase(iterator const it) noexcept(std::is_nothrow_move_assignable_v<T>) {
//     for (iterator i = it + 1; i < end(); ++i) *(i - 1) =
//     std::move_if_noexcept(*i); resize(size() - 1); return const_cast<T *>(it
//     + 1);
// }

void test_erase() {
    qc::vector<int> arr({1, 2, 3, 4, 5, 6, 7});
    arr.erase(arr.begin());
    print<int>("arr", arr);
}

void test_insert() {
    std::cout << "=========== test_insert ===========" << std::endl;
    qc::vector<int> arr{1, 2, 3, 4, 5, 6};
    arr.insert(arr.begin() + 3, {40, 45, 67});
    print<int>("arr", arr);
}

int main() {
    test_vector();

    test_move();

    test_copy_assign();

    // test_thread();

    test_emplace_back();

    // std::cout << sizeof(qc::vector<int>) << std::endl; //use
    // [[no_unique_address]] -> 24, not use -> 32 (一个指针)

    test_erase();

    test_insert();

    return 0;
}