#include <bits/stdc++.h>
using namespace std;

std::vector<std::jthread> pool;

void jthreadfun() {

}

void myfunc() {
    std::jthread t1([&]{
        jthreadfun();
    });
    pool.push_back(std::move(t1));
}

void interact() {
    
}
int main() {

    myfunc();
    interact();

    return 0;
}