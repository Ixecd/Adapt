#include <bits/stdc++.h>
using namespace std;

// 开闭原则,单一职责

// struct 默认 public继承
// clas 默认 private 继承
// template <class T>
struct Reduce {
public:
    virtual int init() = 0;
    virtual int operation(int lhs, int rhs) = 0;
};

// template <class T>
struct PlusReduce : Reduce {
    int init() override { return 0; }
    int operation(int lhs, int rhs) override { return lhs + rhs; }
};

// template <class T>
struct MultiReduce : Reduce {
    int init() override { return 1; }
    int operation(int lhs, int rhs) override { return lhs * rhs; }
};
// loop只需要一个即可,节约改的成本(策略模式)
int loop(vector<int> v, Reduce* Pred) {
    int res = Pred->init();
    for (int i = 0; i < v.size(); ++i) {
        res = Pred->operation(res, v[i]);
    }
    return res;
}

// 多重策略,再定义一个fetch数据
struct Inputer {
    virtual std::optional<int> fetch() = 0;
};

struct CinInputer : Inputer {
    std::optional<int> fetch() override {
        int tmp;
        cin >> tmp;
        while (tmp == -1) return std::nullopt;
        return tmp;
    }
};

struct VectorInputer : Inputer {
    vector<int> v;
    int pos = 0;
    VectorInputer(vector<int> v) : v(v) {}
    std::optional<int> fetch() override {
        if (pos == v.size()) return std::nullopt;
        return v[pos++];
    }
};

int loop(Inputer* input, Reduce* Pred) {
    int res = Pred->init();
    while (auto tmp = input->fetch()) {
        if (tmp.has_value())
            res = Pred->operation(res, tmp.value());
    }
    return res;
}

int main() {
    vector<int> vec{1, 2, 3, 4, 5};
    Reduce* reduce = new PlusReduce();
    std::cout << loop(vec, reduce) << std::endl;
    delete reduce;
    reduce = new MultiReduce();
    std::cout << loop(vec, reduce) << std::endl;
    // 多重策略
    std::cout << loop(new CinInputer(), new PlusReduce()) << std::endl;

    return 0;
}