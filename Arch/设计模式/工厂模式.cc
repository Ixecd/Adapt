#include <bits/stdc++.h>
using namespace std;
// 工厂模式一般只有一个函数->create,return new

// RAII自动管理内存
// 使用unique_ptr<T>代替T*,用make_unique<T>(xxx)代替 new T(xxx)
// shard_ptr可以隐式转换为unique_ptr

// 首先设计策略模式,把不同的子弹类型作为不同的策略传入player函数,造成不同类型的伤害
struct Bullet {
    virtual void explode() = 0;
};

struct Ak47Bullet : Bullet {
    void explode() override { puts("物理伤害"); }
};

struct MagicBullet : Bullet {
    void explode() override { puts("魔法伤害"); }
};

void player(Bullet *bullet) { bullet->explode(); }

// 上面的方法只有一颗子弹,听个响
// 工厂模式的本质是受之于鱼不如授之以渔,给子弹不如给枪
// 工厂模式也可以套工厂给枪不如给造枪场

struct Gun {
    virtual unique_ptr<Bullet> shoot() = 0;
};

struct Ak47Gun : Gun {
    unique_ptr<Bullet> shoot() override { return make_unique<Ak47Bullet>(); }
};

struct MagicGun : Gun {
    unique_ptr<Bullet> shoot() override { return make_unique<MagicBullet>(); }
};

// void player(Gun *gun) {
//     for (int i = 0; i < 2; ++i) {
//         Bullet *bullet = gun->shoot();
//         bullet->explode();
//     }
// }

// 利用模板自动为不同的子弹类型批量定义工厂
template <class B>
struct GunWithBullet : Gun {
    static_assert(is_base_of<Bullet, B>::value, "B 必须是 Bullet 的子类");

    unique_ptr<Bullet> shoot() override { return make_unique<B>(); }
};

void player(Gun *gun) {
    for (int i = 0; i < 5; ++i) {
        auto bullet = gun->shoot();
        bullet->explode();
    }
}

// 超级工厂模式,工厂的工厂
Gun *getGun(string name) {
    if (name == "Ak47") {
        // return new GunWithBullet<Ak47Bullet>();
        return make_unique<GunWithBullet<Ak47Bullet>>().get();
    } else if (name == "Magic") {
        return make_unique<GunWithBullet<MagicBullet>>().get();
    } else {
        throw runtime_error("没有这种枪");
    }
}

// 实战 (分离)
//  将初始化和操作分离
struct ReducerState {
    virtual void operation(int val) = 0;
    virtual int result() = 0;
};
// 是状态的工厂,初始化就是初始化一个操作状态
struct Reducer {
    virtual unique_ptr<ReducerState> init() = 0;
};

struct SumReducerState : ReducerState {
    int res;

    SumReducerState() : res(0) {}

    void operation(int val) override { res = res + val; }

    int result() override { return res; }
};

struct AverageReducerState : ReducerState {
    int res;
    int count;

    AverageReducerState() : res(0), count(0) {}
    void operation(int val) override {
        res = res + val;
        count += 1;
    }

    int result() override { return res / count; }
};

struct SumReducer : Reducer {
    unique_ptr<ReducerState> init() override {
        return make_unique<SumReducerState>();
    }
};

struct AverageReducer : Reducer {
    unique_ptr<ReducerState> init() override {
        return make_unique<AverageReducerState>();
    }
};

// 多重策略,再定义一个fetch数据
struct Inputer {
    virtual std::optional<int> fetch(int val = -1) = 0;
};

struct CinInputer : Inputer {
    // int threshold;

    // void SetThreshold(int thr) {
    //     threshold = thr;
    // }

    std::optional<int> fetch(int val = -1) override {
        int tmp;
        cin >> tmp;
        while (tmp == val) return std::nullopt;
        return tmp;
    }
};

struct VectorInputer : Inputer {
    vector<int> v;
    int pos = 0;
    VectorInputer(vector<int> v) : v(v) {}
    std::optional<int> fetch(int val = 0) override {
        if (pos == v.size()) return std::nullopt;
        return v[pos++];
    }
};

int reduce(Inputer *inputer, Reducer *reducer) {
    unique_ptr<ReducerState> state = reducer->init();
    while (auto val = inputer->fetch()) {
        if (val.has_value()) state->operation(val.value());
    }
    return state->result();
}

// 再来一种更通用的 类似于子弹
struct ProduceBase {
    virtual void use() = 0;
    virtual ~ProduceBase(){}; // 析构函数不能被定义为纯虚函数
};

struct Produce1 : ProduceBase {
    void use() { std::cout << "Produce1 used" << std::endl; }
    ~Produce1() override { std::cout << "~Pro1" << std::endl; }
};

struct Produce2 : ProduceBase {
    void use() { std::cout << "Produce2 used" << std::endl; }

    ~Produce2() override { std::cout << "~Pro2" << std::endl; }
};

struct Factory {
    ProduceBase *getOne(int type) {
        switch (type) {
            case 1:
                return new Produce1();
            case 2:
                return new Produce2();
            default:
                throw out_of_range("No Valid type");
        }
    }
};

int main() {
    // player(new Ak47Bullet());
    // player(new MagicBullet());

    // player(new Ak47Gun());
    // player(new MagicGun());

    // player(make_unique<GunWithBullet<Ak47Bullet>>().get());
    // player(make_unique<GunWithBullet<Ak47Bullet>>().get());

    // player(getGun("Ak47"));
    // player(getGun("Magic"));

    // Inputer *inputer = new CinInputer();
    // Reducer *reducer = new AverageReducer();

    // std::cout << reduce(inputer, reducer);

    Factory factory;
    ProduceBase *base = factory.getOne(1);
    base->use();
    ProduceBase *b = factory.getOne(2);
    b->use();

    ProduceBase *p = factory.getOne(0);
    p->use();

    return 0;
}