#include <bits/stdc++.h>
using ll = long long;
using namespace std;

class C {
public:
    void print() {
        cout << x << endl;
        cout << y << endl;
    }
    int x{1};
    int y{5};
};

void func(C* c) {
    c->print();
}

int main() {

    unique_ptr<C> c = make_unique<C>();
    auto fun = bind(func, c.get());
    fun();

    return 0;
}