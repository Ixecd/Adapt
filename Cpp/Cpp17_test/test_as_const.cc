#include <bits/stdc++.h>
#include <utility> // for std::as_const
using namespace std;


//! std::cout << nullptr ; -> 编译错误

// std::as_const((T)data) -> T const &
// std::move((T)data) -> T &&
class Foo {
public:
    Foo(int val = 0, char* data = nullptr) : m_val(val), m_data(data) {
        std::cout << "foo" << std::endl;
    }
    ~Foo() { std::cout << "~foo" << std::endl; }

    Foo(Foo&& that) noexcept {
        m_data = that.m_data;
        m_val = that.m_val;
        that.m_data = nullptr;
        that.m_val = 0;
    }

    Foo& operator=(Foo const& that) {
        // 自我赋值比较的是对象的地址
        if (&that == this) return *this;
        m_data = that.m_data;
        m_val = that.m_val;
        return *this;
    }

    Foo& operator=(Foo&& that) noexcept {
        m_data = that.m_data;
        m_val = that.m_val;
        that.m_data = nullptr;
        that.m_val = 0;
        return *this;
    }

public:
    int m_val;
    char* m_data;
};

void func(const Foo& f) {
    std::cout << "func" << std::endl;
    std::cout << f.m_val << std::endl;
}

int main() {
    Foo f1(2);

    Foo f2;
    // 将一个非const对象安全的转换为const对象的引用
    f2 = std::as_const(f1);  //->赋值的 为了节省性能 使用常量引用

    func(std::as_const(f1));

    return 0;
}