#define _GLIBCXX_DEUBG
#include <vector>

int main() {

    std::vector<int> arr { 1,2,3,4};
    arr[4]; // ! err 越界
    arr.at(4); // ok throw exception


    return 0;
}