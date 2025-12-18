#include <bits/stdc++.h>
#include <stdio.h>
#include <string.h>

#include <cstring>
using namespace std;

int main() {
    string str = "hello!\n";
    const char *s1 = "hello!\n";

    cout << "str.size() " << str.size() // 7
         << " strlen(s1) = " << strlen("hello!\n") << endl; // 7

    return 0;
}