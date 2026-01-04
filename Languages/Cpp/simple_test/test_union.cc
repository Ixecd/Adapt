#include <bits/stdc++.h>
using namespace std;

typedef void (*hello)(int a, string b);

union Uni {
    int Num; // 4

    struct {
        void *ptr; // 8
        hello foo; // 8
    };
};

int main() {
    cout << "sizeof int = " << sizeof(int) << endl;
    cout << "sizeof void* = " << sizeof(void *) << endl;
    cout << "sizeof hello = " << sizeof(hello) << endl;

    cout << "sizeof Uni = " << sizeof(Uni) << endl;  // 16

    return 0;
}