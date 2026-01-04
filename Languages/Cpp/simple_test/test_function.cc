#include <bits/stdc++.h>

using namespace std;

using MyFun = function<void(int, string)>;

static MyFun foo(int a, string b) {
    cout << "a = " << a << " b = " << b << endl;
    return nullptr;
}

int main() {
    foo(1, "hello");

    return 0;
}