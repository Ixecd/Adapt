#include <bits/stdc++.h>

using namespace std;


using MyFunc = function<void(int id, string name)>;

unordered_map<int, MyFunc> umap;

MyFunc func(int id, string name) {
    cout << id << endl;
    cout << name << endl;
    return nullptr;
}

int main() {

    umap[0] = func;

    umap[0](123, "hello");

    return 0;
}