#include <bits/stdc++.h>
using namespace std;

void Foo() {
    cout << "Foo" << endl;
}

// function 搭配 lambda 使用
function<void()> func = []() { cout << "func" << endl; }; 
function<void()> func2 = Foo;
int main() {
    cout << "main begin" << endl;
    function<void()> cb;
    cb.swap(func);
    cb();
    cb.swap(func2);
    cb();
    //func();

    return 0;
}