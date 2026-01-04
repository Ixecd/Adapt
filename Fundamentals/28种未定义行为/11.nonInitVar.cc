// 检测未定义行为
#define _GLIBCXX_DEBUG
#include <bits/stdc++.h>
#include "qc.hpp"
using namespace std;

// 定义一个变量,不初始化
// int i; --> gcc 大概率 0, msvc 大概率 0xcccccccc / 0xcdcdcdcd / 上一次填写过的值
int main() {

    int i; 

    return 0;
}