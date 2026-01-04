#include <bits/stdc++.h>

#include <future>
using namespace std;

/**
 * @brief std::async 接受一个带返回值的lambda,自身返回一个std::future对象.
 *        lambda 的函数体将在另一个线程里执行,最后调用future的get()方法,
 *        如果此时lambda还没有完成,就会等待 其执行完毕,并获取返回值.
 *        如果调用future的wait方法,只会等并不会返回值
 *        也可以等待一段时间wait_for/wait_until
 */

/**
 * @brief 实现惰性求值的方法:std::launch::deferred 作为参数.
 *        std::async的第一个参数可以设置为std::launch::deferred,这个时候
 *        不会创建一个线程来执行,而是把lambda函数体内的运算推迟到future的get()被调用时.
 *        其只是函数式编程范式意义上的异步,而不涉及到真正的多线程.
 * @details std::async 的底层实现: std::promise
 *          如果不想让std::async
 * 来自动创建线程,想要手动创建线程,可以直接调用std::promise
 *          在线程返回的时候,用set_value()设置返回值.在主线程里,用get_future()获取其std::future对象
 *          进一步get()可以等待并获取线程返回值.
 */

/**
 * @brief std::future小贴士
 *        future为了三五法则,删除了拷贝构造/赋值函数.如果需要浅拷贝,实现共享同一个future对象,可以用std::shared_future.
 *        如果不需要返回值,std::async里lambda的返回类型为void,future对象的类型为std::future<void>
 *        std::promise<void>
 * 其set_value()不接受参数,仅仅作为同步用,不传递任何实际的值.
 */

int main() {
    std::future<int> fut = std::async([&]() -> int { return 1; });
    std::future<int> fut2 =
        std::async(std::launch::deferred, [&]() -> int { return 100; });
    cout << fut.get() << endl;
    cout << fut2.get() << endl;

    std::promise<int> pro;
    std::thread t1([&]() {
        int value = 99;
        pro.set_value(value);
    });
    t1.join();
    std::future<int> fut3 = pro.get_future();
    cout << fut3.get() << endl;

    return 0;
}