#include <bits/stdc++.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>  // for eventfd
using ll = long long;
using namespace std;

class eventBase {
public:
    uint64_t mask;
    function<void()> rdCb;
    function<void()> wtCb;
};

class eventLoop {
public:
    eventLoop() {
        mEpfd = epoll_create1(0);
        mEvents.reserve(16);
        assert(mEpfd != -1);
    }

    int addEvent(int fd, uint64_t event, function<void()> cb) {
        if (!mListened.count(fd)) return -1;
        if (mBase.find(fd)->second->mask & event) return 0;
        epoll_event epevent;
        epevent.events = event;
        epevent.data.fd = fd;
        std::shared_ptr<eventBase> eb = std::make_shared<eventBase>();
        eb->mask = event;
        if (event & EPOLLIN) {
            eb->rdCb = cb;
        } else if (event & EPOLLOUT) {
            eb->wtCb = cb;
        }
        mBase[fd] = eb.get();
        int rt = epoll_ctl(mEpfd, EPOLL_CTL_ADD, fd, &epevent);
        return 1;
    }

    int delEvent(int fd, uint64_t event) {
        if (!mListened.count(fd)) return -1;
        auto it = mBase.find(fd);
        if (it->second->mask & event == 0) return 0;
        it->second->mask &= ~event;
        if (it->second->mask == 0) {
            int rt = epoll_ctl(mEpfd, EPOLL_CTL_DEL, fd, nullptr);
            mBase.erase(fd);
        }
        return 1;
    }

    void process() {
        while (1) {
            int timeOut = 1000;
            int nfds = epoll_wait(mEpfd, mEvents.data(), 16, timeOut);
            if (nfds == 0) cout << "TimeOut" << endl;
            for (int i = 0; i < nfds; ++i) {
                epoll_event epevent = mEvents[i];
                int fd = epevent.data.fd;
                int ev = epevent.events;
                auto eb = mBase.find(fd)->second;
                if (ev & EPOLLIN) {
                    eb->rdCb();
                } else if (ev & EPOLLOUT) {
                    delEvent(fd, EPOLLOUT);
                    eb->wtCb();
                }
            }
        }
    }

    ~eventLoop() { close(mEpfd); }

    eventLoop& operator=(eventLoop&&) = delete;

private:
    int mEpfd;
    vector<epoll_event> mEvents;
    unordered_set<int> mListened;
    unordered_map<int, eventBase*> mBase;
};

using task_func = void (*)(void*);
class taskMsg {
public:
    enum TASK_TYPE {
        NEW_CONN,
        NEW_TASK,
    };

    TASK_TYPE type;

    union {
        int connfd;
        struct {
            task_func task_cb;
            void* args;
        };
    };
};

template <class T>
class threadQueue {
public:
    explicit threadQueue() {
        mEvent = eventfd(0, EFD_NONBLOCK);
        cout << "threadEpfd = " << mEvent << endl;
        assert(mEvent != -1);
    }
    ~threadQueue() { close(mEvent); }
    threadQueue& operator=(threadQueue &&) = delete;

public:
    int send(T const& task) {
        unsigned long long idle_num = 1;
        // unique_lock<mutex> ulk(mLock);
        mQue.push(task);

        int rt = write(mEvent, &idle_num, sizeof(unsigned long long));
        assert(rt != -1);
        return 1;
    }

    int recv(queue<T>& newQueue) {
        unsigned long long idle_num = 1;
        // unique_lock<mutex> ulk(mLock);
        int rt = read(mEvent, &idle_num, sizeof(unsigned long long));
        assert(rt != -1);
        swap(mQue, newQueue);
        return 1;
    }

    void setLoop(eventLoop* loop) { mLoop = loop; }

    void setCb(function<void()> cb) {
        mCb = cb;
        mLoop->addEvent(mEvent, EPOLLIN, cb);
    }

private:
    // 每个线程都有自己独立的消息队列,不需要锁哦
    // mutex mLock;
    eventLoop* mLoop;
    int mEvent;
    function<void()> mCb;
    queue<taskMsg> mQue;
};

void deal_task_message(void* args) {
    threadQueue<taskMsg>* que = (threadQueue<taskMsg>*)args;
    queue<taskMsg> tasks;
    que->recv(tasks);
    while (!tasks.empty()) {
        // 这里直接依赖默认的拷贝构造函数就行
        taskMsg task = tasks.front();
        tasks.pop();

        if (task.type == taskMsg::NEW_CONN) {
            printf("[thread] : get new connection succ!\n");
        } else if (task.type == taskMsg::NEW_TASK) {
            // TODO
            // loop->add_task(task.task_cb, task.args);
        } else {
            fprintf(stderr, "unknow task!\n");
        }
    }
}

void threadMainFun(void* args) {
    // 1. 创建事件循环
    shared_ptr<eventLoop> loop = std::make_shared<eventLoop>();
    assert(loop != nullptr);
    // 2. 绑定消息队列
    threadQueue<taskMsg>* que = (threadQueue<taskMsg>*)args;
    que->setLoop(loop.get());
    que->setCb(bind(deal_task_message, args));
    // 3. 开始事件循环
    cout << "start process" << endl;
    loop->process();
}

class threadPool {
public:
    explicit threadPool(size_t threads = 1) {
        mPool.reserve(threads);
        mSize = threads;
        for (size_t i = 0; i < threads; ++i) {
            mQueues.emplace_back(std::make_shared<threadQueue<taskMsg>>());
            mPool.emplace_back(thread(threadMainFun, mQueues[i].get()));
        }
    }

    ~threadPool() {
        for (size_t i = 0; i < mSize; ++i) mPool[i].join();
    }

    threadPool& operator=(threadPool &&) = delete;

private:
    vector<shared_ptr<threadQueue<taskMsg>>> mQueues;
    vector<thread> mPool;
    size_t mSize;
};

int main() {

    threadPool p(5);

    return 0;
}