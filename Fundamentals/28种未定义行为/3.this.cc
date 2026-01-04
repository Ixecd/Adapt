#define _GLIBCXX_DEBUG
#include <iostream>
#include <memory>
struct C {
    // 默认第一个参数类型为 C const* this
    void print() { this->x = 1; }
    static void print_y() {
        y = 200;
        std::cout << "y = " << y << std::endl;
    }
    int x;
    static int y;
};
int C::y = 100;

int main() {
    C* c = nullptr;
    c->print(); // err 等价于 c->print(c); -> nullptr->x = 1;
    c->print_y();  // ok staitc没有this指针不影响单个类

    return 0;
}