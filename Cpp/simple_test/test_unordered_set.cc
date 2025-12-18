#include <bits/stdc++.h>
using namespace std;

int main() {

    set<int> uset {2,1,3,2,7,5,7,9,0,13,6,17};

    for (auto it = uset.begin(); it != uset.end(); ++it) {
        cout << *it << endl;
    }
    
    return 0;
}