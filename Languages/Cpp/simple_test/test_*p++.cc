#include <bits/stdc++.h>
using namespace std;

int main() {
    int ary[5]{1, 2, 3, 4, 5};
    int *p = ary;
    // *p++先解引用再++
    std::cout << *p++ << std::endl;  // -> 1
    std::cout << *p << std::endl;    // -> 2

    return 0;
}