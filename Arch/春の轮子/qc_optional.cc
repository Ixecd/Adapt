#include <exception>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
namespace qc {

// tag类,表示空
struct nullopt_t {
    explicit nullopt_t() = default;
};
inline constexpr nullopt_t nullopt;

struct in_place_t {
    explicit in_place_t() = default;
};
inline constexpr in_place_t in_place;

class bad_optional_access : public std::exception {
public:
    bad_optional_access() = default;
    virtual ~bad_optional_access() = default;

    const char* what() const noexcept override { return "bad optional access"; }
};

template <class T>
class optional {
public:
    bool _has_value;
    /// @brief cpp中union包含的数据成员不会默认初始化,因为有可能不使用它
    union {
        T _value;
        nullopt_t _nullopt;
    };

public:
    // C++20 explicit(bool(exp))
    explicit(false) optional(T const& value)
        : _value(std::move(value)), _has_value(true) {}

    explicit(false) optional(T&& value)
        : _value(std::move(value)), _has_value(true) {}

    optional() : _has_value(false) {}
    /// @brief 用户使用nullopt调用下面这个函数,nullopt的类型就是nullopt_t
    /// 构造函数 + Tag类来重载构造函数
    optional(nullopt_t) : _has_value(false) {}

    optional(optional const& that) : _has_value(that._has_value) {
        if (_has_value)  // new (&_value) T(that._value);
            // _value = T(that._value) -> T& operator=(T const&)
            std::construct_at(std::addressof(_value), that._value);
    }

    optional(optional&& that) noexcept : _has_value(that._has_value) {
        if (_has_value)
            std::construct_at(std::addressof(_value), std::move(that._value));
    }

    template <class... Ts>
    explicit optional(in_place_t, Ts&&... value_args)
        : _has_value(true), _value(std::forward<Ts>(value_args)...) {}

    // optional<vector<int>> ov({1,2,3,4})
    template <class U, class... Ts>
    explicit optional(in_place_t, std::initializer_list<U> ilist,
                      Ts&&... value_args)
        : _has_value(true), _value(ilist, std::forward<Ts>(value_args)...) {}

    optional& operator=(optional const& that) {
        if (std::addressof(that) == this) return *this;
        if (_has_value) {
            std::destroy_at(std::addressof(_value));
            _has_value = false;
        }
        if (that._has_value) {
            std::construct_at(&_value, that._value);
            _has_value = true;
        }
        return *this;
    }

    optional& operator=(optional&& that) noexcept {
        if (std::addressof(that) == this) return *this;
        if (_has_value) {
            std::destroy_at(std::addressof(_value));
            _has_value = false;
        }
        if (that._has_value) {
            std::construct_at(std::addressof(_value), std::move(that._value));
            _has_value = true;
        }
        that._has_value = false;
        return *this;
    }

    optional& operator=(nullopt_t const&) {
        if (_has_value) std::destroy_at(std::addressof(_value));
        _has_value = false;
        return *this;
    }

    optional& operator=(T const& val) {
        if (_has_value) {
            std::destroy_at(std::addressof(_value));
            _has_value = false;
        }
        std::construct_at(std::addressof(_value), val);
        _has_value = true;
        return *this;
    }
    // T&& 传进来会退化成指针
    optional& operator=(T&& val) noexcept {
        if (_has_value) {
            std::destroy_at(std::addressof(_value));
            _has_value = false;
        }
        std::construct_at(std::addressof(_value), std::move(val));
        _has_value = true;
        return *this;
    }
    // 采用万能引用&& + 完美转发std::forward<Ts> 实现的理论是折叠引用
    template <class... Ts>
    void emplace(Ts&&... value_args) {
        if (_has_value) {
            std::destroy_at(std::addressof(_value));
            _has_value = false;
        }
        // new (&_value) T(value_args...);
        // std::construct_at(&_value,
        // std::forward<decltype(value_args)>(value_args)...);
        std::construct_at(std::addressof(_value),
                          std::forward<Ts>(value_args)...);
        _has_value = true;  // 不能提前开香槟
    }

    void reset() noexcept {
        if (_has_value) {
            std::destroy_at(std::addressof(_value));
            _has_value = false;
        }
        // *this = nullopt;
    }

    ~optional() noexcept {
        if (_has_value) std::destroy_at(std::addressof(_value));
    }

    bool has_value() const noexcept { return _has_value; }

    T const& value() const& {
        if (!_has_value) throw qc::bad_optional_access();
        return std::as_const(_value);
    }

    T& value() & {
        if (!_has_value) throw qc::bad_optional_access();
        return _value;
    }
    // for: std::move(qc::optional<int>(1)).value();
    // auto s1 = std::move(opts.value());
    // auto s2 = std::move(opts).value();
    // 上面两个是等价的
    // 兼容右值 qc::optional<int> parseInt(std::string s);
    // parseInt("42").value() -> 右值
    T const&& value() const&& {
        if (!_has_value) throw qc::bad_optional_access();
        return std::move(_value);
    }

    T&& value() && {
        if (!_has_value) throw qc::bad_optional_access();
        return std::move(_value);
    }

    T const& value_or(T const& default_value) & {
        if (!_has_value) return default_value;
        return _value;
    }

    T const& value_or(T const& default_value) const& noexcept(
        noexcept(T(std::declval<T const&>()))) {
        if (!_has_value) return default_value;
        return _value;
    }
    // 当前对象是右值,马上要弃之不用了,将资源转移
    T&& value_or(T&& default_value) && noexcept(
        noexcept(T(std::declval<T&&>()))) {
        if (!_has_value) return default_value;
        return std::move(_value);
    }

    // 将optional当指针来使用(四个版本)
    T const& operator*() const& {
        if (!_has_value) throw qc::bad_optional_access();
        return _value;
    }
    // 默认走
    T& operator*() & {
        if (!_has_value) throw qc::bad_optional_access();
        return _value;
    }

    T&& operator*() && {
        if (!_has_value) throw qc::bad_optional_access();
        return std::move(_value);
    }

    T const&& operator*() const&& {
        if (!_has_value) throw qc::bad_optional_access();
        return std::move(_value);
    }
    // 标准库中调用 -> 如果没有值是未定义的行为,这里为了安全,设置异常管理
    T* operator->() {
        if (!_has_value) throw qc::bad_optional_access();
        return std::addressof(
            _value);  // 这里的&有可能不工作,如果用户在自己的类(C)中定义了函数
                      // int operator&() const {...}
    }

    T const* operator->() const {
        if (!_has_value) throw qc::bad_optional_access();
        return std::addressof(
            _value);  // 当&运算符被重载的时候,std::addressof会正确取出_value的地址
    }

    // 下面是定义一个隐式转换, 为了避免 int i = opt; 加上explicit 必须强制转换
    // int i = (bool)opt; ok
    explicit operator bool() const noexcept { return _has_value; }

public:
    // --- extension ----
    bool operator==(T const& value) const noexcept {
        if (!_has_value) return false;
        return _value == value;
    }

    bool operator!=(T const& value) const noexcept {
        if (!_has_value) return true;
        return _value == value;
    }

    bool operator==(nullopt_t) const noexcept { return !_has_value; }
    // friend 相当于全局函数没有this指针
    friend bool operator==(nullopt_t, optional const& self) noexcept {
        return !self._has_value;
    }

    bool operator!=(nullopt_t) const noexcept { return _has_value; }
    // friend 相当于全局函数没有this指针
    friend bool operator!=(nullopt_t, optional const& self) noexcept {
        return self._has_value;
    }

    template <class U>
    bool operator==(optional<U> const& that) const noexcept {
        if (_has_value != that._has_value) return false;
        if (_has_value) return _value == that._value;
        return true;
    }

    template <class U>
    bool operator!=(optional<U> const& that) const noexcept {
        return !operator==(that);
    }

    template <class U>
    bool operator>(optional<U> const& that) const noexcept {
        if (!_has_value || !that._has_value) return false;
        return _value > that._value;
    }

    template <class U>
    bool operator<(optional<U> const& that) const noexcept {
        if (!_has_value || !that._has_value) return false;
        return _value < that._value;
    }

    template <class U>
    bool operator<=(optional<U> const& that) const noexcept {
        if (!_has_value || !that._has_value) return false;
        return _value <= that._value;
    }

    template <class U>
    bool operator>=(optional<U> const& that) const noexcept {
        if (!_has_value || !that._has_value) return false;
        return _value >= that._value;
    }

    template <class F>
    auto and_then(F&& f) {
        // decay_t 去引用 主要是针对参数的,对返回类型并没有那么严格 int[] ->
        // int*   int &-> int 更严格应该使用std::remove_cvref_t C++20引入的
        using RetType = std::remove_cvref_t<decltype(f(_value))>;
        // std::remove_cv_t<std::remove_reference_t<decltype(f(_value))>>;
        if (_has_value) {
            return std::forward<F>(f(_value));
        } else
            return RetType{};
    }

    // F 可能是lambda会捕获环境变量 std::unique_ptr<int>up -> 捕获到这个
    // 使用万能引用,一进来
    template <class F>
    auto transform(F&& f)
        -> optional<
            std::remove_cv_t<std::remove_reference_t<decltype(f(_value))>>> {
        if (_has_value)
            return std::forward<F>(f(_value));
        else
            return nullopt;
    }

    // 相比于value_or or_else性能上更高一点,延迟调用
    template <class F>
    optional or_else(F&& f) {
        if (!_has_value)
            return std::forward<F>(f());
        else
            return *this;
    }
    // using std::swap 
    // C++ ADL机制argument dependent lookup
    // 以此可以实现多态,在namespace中加成员函数,可以实现多态
    // 如果在自己的namespace中有swap,再自己的namespace中的class有swap就会调用自己namespace中的swap
    // namespace MySpace {
    //     class MyClass() {};
    //     void doSomething(MyClass) {}
    // }
    // MySpace::MyClass obj;

    // int func() {
    //     // MySpace::doSomething(obj);
    //     doSomething(obj); //right, 自动从参数的名字空间中找,并不一定是全局的
    // }

    void swap(optional &that) noexcept {
        if (_has_value && that._has_value) {
            using std::swap; // ADL
            // 如果这里的_value 的类型为C,C在namespace MyC中,根据ADL就会去找MyC中是否有swap有的话就用MyC中的,没有的话就用std::swap
            swap(_value, that._value); // 判断自己有没有实现,有的话用自己的,没有的话用std里的
        } else if (!_has_value && !that._has_value) return;
        else if (_has_value) {
            that.emplace(std::move(_value));
            reset();
        } else {
            emplace(std::move(that.m_value));
            that.reset();
        }
    }

void swap(optional<int> &lhs, optional<int> &rhs) {
    printf("qc::swap()...");
    std::swap(lhs, rhs);
}


};

#if __cpp_deduction_guides
template <class T>
optional(T) -> optional<T>;
#endif

}  // namespace qc

#include <iostream>
namespace MyC {

struct C {
    C(int x, int y) : m_x(x), m_y(y) {
        std::cout << "C(int, int)" << std::endl;
    }

    C(C const& that) : m_x(that.m_x), m_y(that.m_y) {
        std::cout << "C(C const&)" << std::endl;
    }

    C(C&&) { std::cout << "C(C &&)" << std::endl; }

    ~C() { std::cout << "~C()" << std::endl; }

    int operator&() const { return m_x; }

    C & operator=(C && that) {
        m_x = that.m_x;
        m_y = that.m_y;
        std::destroy_at(&that.m_x);
        std::destroy_at(&that.m_y);
    }

    int value() const { return m_x; }

    int m_x;
    int m_y;
};

void swap(C &lhs, C &rhs) {
    std::cout << "MyC::swap()" << std::endl;
    // 要自定义移动赋值和移动拷贝
    std::swap(lhs, rhs);
}

}

using namespace MyC;
void test_union() {
    union U {
        ~U() {}
        C c;
        int i;
    } u = {.i = 1};

    new (std::addressof(u.c)) C(1, 2);  // ok
    // C++17 CTAD
    // qc::optional(100);
    // optional(T) -> optional<T>

    std::cout << u.c.m_x << " " << u.c.m_y << std::endl;

    u.c.~C();
}

void test_base() {
    qc::optional<int> opt(2);
    // 隐式转换了,将qc::nullopt转换为option类型
    opt = qc::nullopt;

    std::cout << opt.has_value() << std::endl;

    std::cout << opt.value() << std::endl;
}

void test_operator() {
    qc::optional<int> opt(1);

    opt = qc::nullopt;
    std::cout << opt.has_value() << std::endl;  // 0

    opt = 42;

    std::cout << opt.has_value() << std::endl;  // 1
    std::cout << opt.value() << std::endl;      // 42
    std::cout << opt.value_or(0) << std::endl;  // 42

    opt = qc::optional<int>(33);

    std::cout << opt.has_value() << std::endl;  // 1
    std::cout << opt.value() << std::endl;      // 33
    std::cout << opt.value_or(0) << std::endl;  // 33

    opt = qc::optional<int>(qc::nullopt);
    std::cout << opt.has_value() << std::endl;  // 0
}

qc::optional<int> parseInt(std::string s) {
    try {
        return std::stoi(s);
    } catch (...) {
        return qc::nullopt;
    }
}

void test_emplace() {
    // emplace少了一次移动拷贝
    // C (int, int)
    // C (C&&)
    //  ~C()
    //  ~C()
    // emplace就是为了就地构造,防止 C(C&&) (如果类C非常庞大成员变量非常多)
    qc::optional<C> optc = qc::nullopt;
    // optc = std::move(C(1, 2));
    optc.emplace(1, 2);  // optional::emplace(int x, int y) { construct_at...}
    // C++17
    // 所有版本if中的变量都会强制转换为bool类型
    // if (auto opt = parseInt("42"); opt) {
    if (auto opt = parseInt("42")) {
        std::cout << *opt << std::endl;
    } else
        std::cout << "error" << std::endl;
}

void test_pointer() {
    qc::optional<C> opt = qc::nullopt;
    opt.emplace(2, 3);
    // *opt要返回引用
    int x = (*opt).value();
    int y = opt->m_y;
    // -> 支持
    std::cout << "opt's xvalue = " << x << std::endl;
    std::cout << "opt's yvalue = " << y << std::endl;
}

void test_as_bool() {
    qc::optional<int> opt;
    opt = 100;
    if (opt)
        std::cout << "opt is not nullopt" << std::endl;
    else
        std::cout << "opt is nullopt" << std::endl;
    opt = qc::nullopt;
    if (opt)
        std::cout << "opt is not nullopt" << std::endl;
    else
        std::cout << "opt is nullopt" << std::endl;

    // ! int i = opti; 不会发生任何错误
    int i = (bool)opt;
    std::cout << "i = " << i << std::endl;  // int i = (int)((bool)opt)
}

void test_inplace_vector() {
    qc::optional<std::vector<int>> ov(qc::in_place, {1, 2, 3});
    std::cout << ov.has_value() << std::endl;
}

// std::vector<string> v;
// void feed(std::string &&s) {
//     throw qc::bad_optional_access();
//     v.push_back(std::move(s));
// }
// std::string s;
// feed(std::move(s));

/// 注意上面的feed的参数类型,如果是&&传递的是引用,调用feed会类型退化成指针,就算throw也不会影响外面的s
/// 如果是值传递,使用std::move(s)不会退化,直接将所有权转交给feed的形参,throw之后外面的就用不了

int main() {
    // qc::optional<C> opt(C(1, 2));
    // 这里optional<C>
    // 使用nullopt来初始化,里面仍然会调用_value()->默认构造函数,C里面没有->union可以不初始化
    // qc::optional<C> opt2(qc::nullopt);

    // std::cout << opt.has_value() << std::endl;

    // std::cout << opt.value() << std::endl;

    // test_union();

    // test_operator();

    // test_emplace();

    // test_pointer();

    // test_as_bool();

    test_inplace_vector();

    return 0;
}