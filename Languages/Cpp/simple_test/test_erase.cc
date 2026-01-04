#include <bits/stdc++.h>
using namespace std;

int main() {
    vector<int> ary(10, 1);
    cout << ary.size() << endl;
    for (int i = 0; i < 10; ++i) ary[i] = i + 1;
    for (int num : ary) cout << num << " ";
    cout << endl;
    auto it = ary.begin();

    cout << *it << endl;
    ary.erase(it);

    if (it.base() == nullptr)
        std::cout << "cur it is nullptr" << std::endl;
    else
        cout << "after erase(it), *it = " << *it << endl;  // 2
    //  else std::cout << "cur it -> value = " << it->first << std::endl;
    return 0;
}