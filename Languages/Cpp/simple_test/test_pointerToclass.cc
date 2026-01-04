#include <bits/stdc++.h>
using namespace std;

class A {
public:
    A(int id, string name) : m_id(id), m_name(name) {}
    const int getId() const { return m_id; }
    const string getName() const { return m_name; }
    static void setName(string& name);

private:
    int m_id;
    string m_name;
};

static A* t_A = nullptr;

void A::setName(string& name) {
    if (name.empty()) return;
    if (t_A) t_A->m_name = name;
}

int main() {
    A a(1, "NiHao");
    t_A = &a;
    cout << a.getName() << endl; // NiHao
    string name = "EXCHANGE";
    t_A->setName(name); 
    cout << a.getName() << endl; // EXCHANGE 修改成功,表示指针不受访问权限控制

    return 0;
}