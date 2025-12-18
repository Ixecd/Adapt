#include <bits/stdc++.h>

#include "qc.hpp"
using namespace std;

constexpr size_t n = 1 << 26;

int main() {
    vector<int> vec(n);

    TICK(write_1);
    for (size_t i = 0; i < vec.size(); i++) vec[i] = 1;
    TOCK(write_1); // 230ms

    TICK(write_0);
    for (size_t i = 0; i < vec.size(); i++) vec[i] = 0;
    TOCK(write_0); // 230ms

    return 0;
}