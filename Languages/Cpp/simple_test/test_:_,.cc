#include <bits/stdc++.h>
using namespace std;

int main() {
    int op = 1, mask = 0, arg1 = 7, arg2 = 10;
    // 防御性编程->该带的括号就得带上
    op = mask != 0 ? (op + arg1, op + arg2) : (op + arg2, op + arg1);

    cout << op << endl;  // 8

    return 0;
}