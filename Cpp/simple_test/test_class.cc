#include <bits/stdc++.h>
using namespace std;

class A {
public:
    A(int num = 8, bool flag = false);

    void test(int num = 10, bool flag = false);

    const int getNum() const;

    const bool getFlag() const;

private:
    int m_num = 0;
    bool m_flag = false;
};
/// @brief 默认参数,函数的声明和定义中只能有一个,而且要从右往左默认
/// @param num
/// @param flag
A::A(int num, bool flag) {
    m_num = num;
    m_flag = flag;
}

const int A::getNum() const { return this->m_num; }

const bool A::getFlag() const { return this->m_flag; }

int main() {
    A* a = new A();

    cout << "a->flag = " << a->getFlag() << endl;

    cout << "a->num = " << a->getNum() << endl;

    return 0;
}