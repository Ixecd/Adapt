#include <bits/stdc++.h>
#include <coroutine>
using namespace std;

template <class T>
coroutine<int> func(int &sum) {
    sum += 10;
    co_yield sum;
    sum++;
}

int main() {

    int sum = 0;
    ++sum;
    func<int>(sum);
    ++sum;

    return 0;
}