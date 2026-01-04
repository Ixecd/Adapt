#include <iostream>
#include "../无注释臻享版/vector.hpp"

using namespace qc;

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