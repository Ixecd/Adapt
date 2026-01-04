// 运行时检测
#define _GLIBCXX_DEBUG
#include <vector>
#include <memory>

// 常量表达式规定编译器期间不能出现任何错误,如果出现未定义行为直接会编译器报错
constexpr void func() {
    std::unique_ptr<int> p = nullptr;
    *p;
}

int main() {
    std::vector<int> arr{1, 2, 3, 4, 5, 6};
    int *end = &*(arr.end());

    //建议如果非要对象指针的话
    int* begin = arr.data();
    int* end = arr.data() + arr.size();

    func();

    return 0;
}