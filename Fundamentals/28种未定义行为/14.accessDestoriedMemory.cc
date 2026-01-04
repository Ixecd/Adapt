#define _GLIBCXX_DEBUG

int main() {

    int* p = new int{};
    delete p;
    // p = nullptr; //good
    int x = 100;
    *p = 100; // error!

    return 0;
}