#include <bits/stdc++.h>
using namespace std;

int main() {
    // float fres = 0;

    // ostringstream oss("");

    // oss << fres << " " << "hello";

    // string str = oss.str();

    // cout << str << endl; // 0 hello

    // float fresult;

    // ostringstream oss1;

    // oss1 << fresult << " " << "N";

    // cout << oss1.str() << endl;

    string str = "3.14 100 9.99";

    istringstream ss(str);

    int f1;
    int f2;
    double f3;

    ss >> f1 >> f2 >> f3;
    cout << f1 << " " << f2 << " " << f3;

    return 0;
}