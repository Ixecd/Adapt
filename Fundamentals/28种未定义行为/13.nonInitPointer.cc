#define _GLIBCXX_DEBUG
#include "qc.hpp"

struct Dog {
    int age;
};

struct Person {
    Dog* dog;
};

int main() {
    int* q;
    *q;  // error!

    Person* p = new Person;
    p->dog->age;  // error!

    return 0;
}