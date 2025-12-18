#include <bits/stdc++.h>
using namespace std;

struct Foo {
    int val;
    int* ptr = nullptr;
    Foo() {
        ptr = new int{1};
        cout << "Foo() construct..." << endl;
    }
    ~Foo() {
        if (ptr != nullptr) {
            delete ptr;
            cout << "~Foo() destruct..." << endl;
        }
        ptr = nullptr;
    }

    Foo(Foo&& that) {
        ptr = that.ptr;
        val = that.val;
        that.ptr = nullptr;
        that.val = 0;
    }

    Foo(Foo const& that) {}

    Foo& operator=(Foo&& that) {
        if (this == &that) return *this;
        if (ptr != nullptr) delete ptr;
        ptr = that.ptr;
        val = that.val;
        that.val = 0;
        that.ptr = nullptr;
        return *this;
    }
};

struct Goo {
    int val;
    Goo() { cout << "Goo() construct..." << endl; }
    ~Goo() { cout << "~Goo() destruct..." << endl; }
};

// 构造初始化
// std::construct_at(&addr,
// data)在指定的内存地址上构造对象。它是为了提供一种类型安全的方式来在未初始化的内存上构造对象，特别是在需要显式控制对象的构造和析构时
// 和 placement new 非常类似
int main() {
    allocator<Foo> alloc;
    Foo* data = alloc.allocate(1);
    Foo fo;
    // construct_at的第二个参数是引用类型,如果直接传进去会发生,拷贝构造,如果以move的形式传进去会发生移动构造
    construct_at(data, std::move(fo));

    // construct_at 是引用
    // if (fo.ptr == data->ptr) cout << "move" << endl;

    cout << "before destroy" << endl;
    destroy_at(data);
    cout << "after destroy" << endl;
    alloc.deallocate(data, 1);
    cout << "after deallocate" << endl;
    return 0;
}

// 总结: 对于一个自定义类,如果里面涉及了内存分配,那么一定要给出 自定义析构函数/拷贝构造函数/移动构造函数/移动赋值函数/拷贝赋值函数