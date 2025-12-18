#include <concepts>
#include <iostream>
#include <iterator>
#include <vector>

template <std::random_access_iterator InputIt>
void test_rai(InputIt first, InputIt last) {
    for (; first != last; ++first) std::cout << *first << " ";
    std::cout << std::endl;
}

template <class T>
concept m_integer = std::is_integral_v<T>;

template <m_integer INT_TYPE>
void print_int(INT_TYPE val) {
    std::cout << val << std::endl;
}

template <class T>
concept m_float = std::is_floating_point_v<T>;

template <m_float FLOAT_TYPE>
void print_float(FLOAT_TYPE val) {
    std::cout << val << std::endl;
}

int main() {
    std::vector<int> vec{1, 2, 3};
    test_rai(vec.begin(), vec.end());

    print_int(100);

    print_float(3.14f);

    return 0;
}