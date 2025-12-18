#include <bits/stdc++.h>
using namespace std;

int main() {
    int a = 10;
    // const int a = 10;
    int const *p = &a;
    // const int *q = const_cast<const int *>(p); 给p添加const 同时左值也得是const
    int *q = const_cast<int *>(p); // 给p去掉const 能不能修改值还要看变量本身是否是const
    *q = 100;
    std::cout << "a = " << a << std::endl;  // 100
    return 0;
}