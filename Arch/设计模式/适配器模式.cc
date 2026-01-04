#include <bits/stdc++.h>
using namespace std;
// 策略模式 + 适配器模式 非常常用

struct Reduce {
public:
    virtual int init() = 0;
    virtual int operation(int lhs, int rhs) = 0;
};

struct PlusReduce : Reduce {
    int init() override { return 0; }
    int operation(int lhs, int rhs) override { return lhs + rhs; }
};

struct MultiReduce : Reduce {
    int init() override { return 1; }
    int operation(int lhs, int rhs) override { return lhs * rhs; }
};
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

// 满足依赖倒置原则
// 满足开闭原则
// 适配器模式(如果-1的时候要终止,那如果0呢)
struct StopInputerAdapter : Inputer {
    Inputer *inputer;
    int stopMark;
    StopInputerAdapter(Inputer *inputer, int stopMark) : inputer(inputer), stopMark(stopMark) {}

    optional<int> fetch() override {
        auto tmp = inputer->fetch();
        if (tmp == stopMark) return nullopt;
        return tmp;
    }
};


// 如果我们还想实现过滤,负数直接丢掉呢
struct FilterInputerAdapter1 : Inputer {
    Inputer* inputer;
    FilterInputerAdapter1(Inputer *inputer) : inputer(inputer) {}
    
    std::optional<int> fetch() override {
        while (true) {
            auto tmp = inputer->fetch();
            if (tmp.has_value()) {
                // 这里如果不要0呢
                if (tmp.value() >= 0) return tmp.value();
            } else return std::nullopt;
        }
    }
};
// Filter中的条件不应该写死,而是应该传入一个FilterStrategy,允许用户扩展
struct FilterStrategy {
    // virtual bool shouldDrop(int value) {
    //     return value == stopValue;
    // }
    virtual bool shouldPass(int value) = 0;
};

// 大于一定值(threshold)才能通过
struct FilterStrategyAbove : FilterStrategy {
    int threshold;

    FilterStrategyAbove(int threshold) : threshold(threshold) {}

    virtual bool shouldPass(int value) {
        return value > threshold;
    }

};

// 小于一定值(threshold)才能通过
struct FilterStrategyBelow : FilterStrategy {
    int threshold;

    FilterStrategyBelow(int threshold) : threshold(threshold) {}

    virtual bool shouldPass(int value) {
        return value < threshold;
    }

};

// 更改后的FilterInputerAdapter
struct FilterInputerAdapter : Inputer {
    Inputer *inputer;
    FilterStrategy* strategy;

    FilterInputerAdapter(Inputer* inputer, FilterStrategy* strategy) : inputer(inputer), strategy(strategy) {}

    std::optional<int> fetch() override {
        while (1) {
            auto tmp = inputer->fetch();
            if (!tmp.has_value()) {
                return std::nullopt;
            } 
            if (strategy->shouldPass(tmp.value())) return tmp.value();
        }
    }
};

// 上面实现的是适配器的策略
// 我们还可以实现适配器的策略的适配器, 实现过滤多个条件的值
struct FilterStrategyAnd : Inputer {
    FilterStrategy* first;
    FilterStrategy* second;

    FilterStrategyAnd(FilterStrategy* first, FilterStrategy* second) : first(first), second(second) {}

    virtual bool shouldPadd(int value) {
        return first->shouldPass(value) && second->shouldPass(value);
    }
};

int main() {

    

    return 0;
}