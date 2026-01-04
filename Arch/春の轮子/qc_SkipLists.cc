#include <iostream>
#include <list>
#include <random>
#include <vector>

using namespace std;

struct SkipListNode {
    int key;
    int value;
    /// @brief 节点的层数由forward的大小决定,这里forward[i]表示第i层 p = p->forward[i] 表示在同一层往下遍历
    vector<SkipListNode*> forward;

    SkipListNode(int k, int v, int level)
        : key(k), value(v), forward(level, nullptr) {}
};

class SkipList {
private:
    int maxLevel;
    int level;
    /// @brief 顶层
    SkipListNode* head;

public:
    SkipList(int maxLevel) : maxLevel(maxLevel), level(0) {
        // ---------------------------------
        head = new SkipListNode(numeric_limits<int>::min(),
                                numeric_limits<int>::min(), maxLevel);
    }
    /// @brief 生成随机level 50%
    /// @return level
    int randomLevel() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, maxLevel);
        int level = 1;
        while (level < maxLevel &&
               dis(gen) < (0.5 * numeric_limits<int>::max())) {
            level++;
        }
        return level;
    }

    // 插入节点
    void insert(int key, int value) {
        // 创建一个 vector 类型的 update 数组，大小为跳表的最大层级，初始化为nullptr
        vector<SkipListNode*> update(maxLevel, nullptr);
        // 从跳表的顶层开始，逐层向下遍历，找到每一层中小于要插入节点的键值的最接近节点，并将这些节点记录在update 数组中
        SkipListNode* p = head;
        for (int i = level - 1; i >= 0; --i) {
            while (p->forward[i] != nullptr && p->forward[i]->key < key) {
                p = p->forward[i];
            }
            // 当前要往下走的一个节点
            update[i] = p;
        }
        // 找到每一层中小于要插入节点的键值的最接近节点后，更新update数组中的节点指针
        p = p->forward[0];
        // 如果 p 为空或者 p 的键值不等于要插入的键值，则表示要插入新节点
        if (p == nullptr || p->key != key) {
            // 随机生成节点的层级
            int newLevel = randomLevel();
            // 如果新节点的层级大于当前跳表的层级，则更新跳表的层级为新节点的层级
            if (newLevel > level) {
                for (int i = level; i < newLevel; ++i) {
                    update[i] = head;
                }
                level = newLevel;
            }
            // 创建新节点
            SkipListNode* newNode = new SkipListNode(key, value, newLevel);
            // 逐层连接新节点到跳表中
            for (int i = 0; i < newLevel; ++i) {
                newNode->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = newNode;
            }
        }
    }

    // 查找节点
    int search(int key) {
        SkipListNode* p = head;
        for (int i = level - 1; i >= 0; --i) {
            while (p->forward[i] != nullptr && p->forward[i]->key < key) {
                p = p->forward[i];
            }
        }
        p = p->forward[0];
        if (p != nullptr && p->key == key) {
            return p->value;
        } else {
            return -1;
        }
    }
};

int main() {
    // 创建一个最大层级为3的跳表
    SkipList skipList(3);

    // 插入一些节点
    skipList.insert(1, 10);
    skipList.insert(2, 20);
    skipList.insert(3, 30);
    skipList.insert(4, 40);

    // 查找节点
    cout << skipList.search(3) << endl;  // 输出 30
    cout << skipList.search(5) << endl;  // 输出 -1

    return 0;
}
