#pragma once

#include <chrono>
#include <iostream>

#define TICK(x) auto bench_##x = std::chrono::steady_clock::now();
#define TOCK(x) std::cout << #x " : " << std::chrono::duration_cast<std::chrono::duration<double, std::milli>> (std::chrono::steady_clock::now() - bench_##x).count() << " ms" << std::endl;

#define PRINT(x) std::cout << #x " : " << x << std::endl;
#define PRINT_S(x) std::cout << #x << std::endl;

template <class Continer>
void PRINT_FOR(Continer const& c) {
    auto it = c.begin();
    if (it != c.end()) std::cout << '{' << *it;
    for (++it; it != c.end(); ++it) 
        std::cout << ", " << *it;
    std::cout << '}' << std::endl;
}