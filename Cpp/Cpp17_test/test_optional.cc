#include <optional>
#include <iostream>

int main() {

    std::optional<int> opt(1);
    std::cout << opt.has_value() << '\n';
    std::cout << opt.value() << '\n';
    std::cout << opt.value_or(0) << '\n';

    std::optional<int> opt2(std::nullopt);
    std::cout << opt2.has_value() << '\n';
    try {
        opt2.value();
    } catch (std::bad_optional_access const &) {
        std::cout << "opt2 exception ok\n";
    }
    std::cout << opt2.value_or(0) << '\n';

    opt2 = 42;
    std::cout << opt2.has_value() << '\n';
    std::cout << opt2.value() << '\n';
    std::cout << opt2.value_or(0) << '\n';

    return 0;
}