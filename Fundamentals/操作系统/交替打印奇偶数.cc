#include <bits/stdc++.h>

#include <condition_variable>
using ll = long long;
using namespace std;
using namespace chrono_literals;
int main() {
    int total = 100;
    atomic<size_t> now = 1;
    mutex mtx;
    condition_variable cv;
    thread t1([&] {  // print even
        while (now < total) {
            unique_lock<mutex> ulk(mtx);
            // wait会自动释放锁,之后被唤醒的时候再次拿锁
            if (now % 2 == 1) cv.wait(ulk);
            cout << "Even : " << now << endl;
            now++;
            cv.notify_one();
            // this_thread::sleep_for(400ms);
        }
    });

    thread t2([&] {  // print odd
        while (now < total) {
            unique_lock<mutex> ulk(mtx);
            if (now % 2 == 0) cv.wait(ulk);
            cout << "Odd : " << now << endl;
            now++;
            cv.notify_one();
            // this_thread::sleep_for(500ms);
        }
    });

    t1.join();
    t2.join();

    return 0;
}