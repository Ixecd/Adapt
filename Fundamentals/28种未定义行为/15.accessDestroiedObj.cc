#define _GLIBCXX_DEBUG
#include "qc.hpp"

struct C {
    int i;
    ~C() { i = 0; }
};

constexpr void func() {
    C *c = (C *)malloc(sizeof(C));
    c->~C();
    c->i;  // error;
    free(c);
}

int main() {

    func();

    return 0;
}