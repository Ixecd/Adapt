#include <cstring>
#include <iostream>
#include <list>
#include "../无注释臻享版/list.hpp"

using namespace qc;

template <class T>
void print(const std::string &name, qc::list<T> const &lst) {
    std::cout << "print : " << name << std::endl;
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

void test_print() {
    std::cout << "test_print initializer_list" << std::endl;
    print<int>("test_print ", qc::list<int>{1, 2, 3, 4, 5});
}

void test_construct_iterator() {
    std::cout << "========= test_construct_iterator =========" << std::endl;
    std::list<int> slst({1, 2, 3, 4});
    qc::list<int> lst(slst.begin(), slst.end());
    print<int>("construct_iteartor lst", lst);
}

void test_initializer_list() {
    std::cout << "========= test_initializer_list =========" << std::endl;
    qc::list<int> lst{1, 2, 3, 4};
    print<int>("inilializer_list lst", lst);
}
// equal to print but in class
void test_foreach() {
    qc::list<int> lst{2, 3, 4};
    print<int>("test_foreach", lst);
    size_t i = 0;
    lst.foreach ([&](int &val) {
        std::cout << "lst[" << i << "].val = " << val << std::endl, ++i;
    });
}

void test_rtraversal() {
    qc::list<int> lst{1, 2, 3};
    print<int>("before reverse tarversal lst", lst);
    std::cout << "after reverse : " << std::endl;
    for (auto it = lst.rbegin(); it != lst.rend(); ++it) {
        int &val = *it;
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

void test_detach_pointer_val() {
    qc::list<int> lst({1, 2, 3, 4});
    print<int>("test_detach_pointer_val lst ", lst);
    std::cout << "after insert val sizeof(ListNode) = "
              << sizeof(*lst.get_dummy()) << std::endl;
}

void test_emplace() {
    std::cout << "======= test_emplace_back =======" << std::endl;
    qc::list<int> lst{1, 2, 3, 4, 5};
    lst.emplace_back(9);
    print<int>("after emplace_back 9 ", lst);
    std::cout << "======= test_emplace_front =======" << std::endl;
    lst.emplace_front(9);
    print<int>("after emplace_front 9 ", lst);
}

void test_erase() {
    std::cout << "======= test_erase =======" << std::endl;
    qc::list<int> lst{1, 2, 3, 4, 5};
    lst.erase(lst.begin());
    print<int>("after erase begin ", lst);
    std::cout << "======= test_erase_bt =======" << std::endl;
    lst.erase(lst.begin(), lst.end());
    print<int>("after erase begin end ", lst);
}

void test_size() {
    std::cout << "======= test_size =======" << std::endl;
    qc::list<int> lst{1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::cout << "lst.size() = " << lst.size() << std::endl;
}

int main() {
    std::cout << "sizeof(qc::list<int>) : " << sizeof(qc::list<int>)
              << std::endl;

    // test_construct_iterator();

    // test_initializer_list();

    // test_foreach();

    // test_rtraversal();

    // test_print();

    // test_detach_pointer_val();

    // test_emplace();

    // test_erase();

    test_size();

    return 0;
}