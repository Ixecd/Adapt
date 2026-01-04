#include <memory>

int func() {
    int x = 100;
} 
/* 返回类型不为 void 的函数, 必须有return函数 */
/* 解决办法: g++ -Werror=return-type */
/* -Werror 表示所有警告都会转换为错误 */
/* -Werror=return-type 表示如果没有写return才会返回错误 */
int main() {

    int x = func();

    return 0;
}