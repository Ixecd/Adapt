#include <iostream>
#include <stdexcept>

void check_and_throw(bool exp) {
    if (exp) [[unlikely]] {
        throw std::out_of_range("Index out of range");
    } else [[likely]]
        throw std::logic_error("just test");
}

int main() {
    try {
        check_and_throw(false);  // 这个调用将抛出异常
    } catch (const std::out_of_range& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }
    return 0;
}
