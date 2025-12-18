#include <bits/stdc++.h>
using namespace std;

int main() {
    vector<int> vec;
    vec.resize(5);
    for (size_t i = 0; i < 5; ++i) vec[i] = i;
    // resize 会保留原来的值, 后面指定的值是新加的值
    vec.resize(10, 1);
    for (size_t i : vec) cout << i << " ";
    cout << endl;

    return 0;
}