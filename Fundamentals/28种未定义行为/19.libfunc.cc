#define _GLIBCXX_DEBUG
#include "qc.hpp"
#include <cstring>
#include <ctype.h> // 里面的一系列字符参数,必须在0~127范围内(即只支持ASCII字符)
int main() {

    auto flag = isdigit('0'); // ok true
    flag = isdigit('a'); // ok true;
    flag = isdigit('\xef'); // err 未定义行为 MSVC 的 Debug模式下会产生断言以常

    char s[] = "你好A"; // UTF-8 编码的中文 EB FE.. 
    std::transform(std::begin(s), std::end(s), std::begin(s), ::tolower); //err ! UTF-8编码会产生大于128的字节
    // 不能得到 "你好a"

    // -1 是为了兼容 EOF 兼容C语言的IO
    // MSVC STL: assert(-1 <= c && c < 256); 
    // 理伦上STL: assert(0  <= c && c <= 127);

    // 解决办法
    // MSVC : 0 ~ 65535
    // GCC  : 0 ~ 0x010fffff
    flag = iswdigit('你'); // ok

    // 建议自己实现
    char c = '2';
    if ('0' <= c && c <= '9'); 
    if (strchr(" \n\t\r", c)); // 代替 isspace(c)


    return 0;
}