#include <bits/stdc++.h>
using namespace std;

void test_FILE_LINE() {
    cout << "test_FILE_LINE start" << endl;
    throw logic_error("logic error");

    cout << endl;
}

int main() {
    test_FILE_LINE();

    return 0;
}