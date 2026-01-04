#include <tbb/parallel_invoke.h>

#include <chrono>
#include <iostream>
#include <string>
void download(std::string file) {
    for (int i = 0; i < 10; ++i) {
        std::cout << "Downloading " << file << " (" << i * 10 << "%)..."
                  << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "Download complete: " << file << std::endl;
}

void interact() {
    std::string name;
    std::cin >> name;
    std::cout << "Hi, " << name << std::endl;
}

int test() {
    // 接受任意数量的lambda函数,自动wait
    tbb::parallel_invoke([&] { download("NV.zip"); }, [&] { interact(); });

    return 0;
}

int better_test() {
    std::string s = "Hello, world!";
    for (size_t i = 0; i < (1 << 26); ++i) s += ' ';

    char ch = 'd';

    auto begin = std::chrono::steady_clock::now();
    tbb::parallel_invoke(
        [&] {
            for (size_t i = 0; i < s.size() / 2; ++i)
                if (s[i] == ch) std::cout << "found !" << std::endl;
        },
        [&] {
            for (size_t i = s.size() / 2; i < s.size(); ++i)
                if (s[i] == ch) std::cout << "found !" << std::endl;
        });
    auto end = std::chrono::steady_clock::now();
    using double_ms = std::chrono::duration<double, std::milli>;
    std::cout << "parallel thread spend time : "
              << std::chrono::duration_cast<double_ms>(end - begin).count()
              << " ms" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] == ch) std::cout << "found !" << std::endl;
    end = std::chrono::steady_clock::now();
    std::cout << "single thread spend time : "
              << std::chrono::duration_cast<double_ms>(end - begin).count()
              << " ms" << std::endl;

    return 0;
}

int main() {
    better_test();

    return 0;
}