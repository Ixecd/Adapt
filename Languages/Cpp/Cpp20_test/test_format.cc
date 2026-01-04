#include <bits/stdc++.h>

#include <format>
using namespace std;

int main() {
    int number = 10;
    string buffer;
    format_to(std::back_inserter(buffer), "number is {}", 100);
    cout << buffer << endl;

    return 0;
}