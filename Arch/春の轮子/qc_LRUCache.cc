#include <chrono>
#include <functional>
#include <iostream>
#include <list>
#include <thread>
#include <unordered_map>
using namespace std;
/**
 * @brief LRUCache 缓存器
 * @details     key->{value, time}
 *          搞清楚 时间点 和 时间段 的关系
 *          unoreder_map<int, pair<int, chrono::system_clock::time_point>> cache
 *              only for key
 *          list<int> lruLists;1
 * 
 *          int capacity;
 * @brief get(int key), put(int key, int value, chrono::seconds t);
 */
class LRUCache {
public:
    LRUCache(int capacity) : capacity(capacity) {}
    /**
     * @brief 
     * 
     * @param key 
     * @return -2 not exist -1 error 0 success 
     */
    int del(int key) {
        auto it = cache.find(key);
        if (it == cache.end()) return -2;
        cache.erase(it);
        lruLists.erase(find(lruLists.begin(), lruLists.end(), key));
        return 0;
    }


    int delAll() {
        cache.clear();
        lruLists.clear();
        return 0;
    }

    int get(int key) {
        auto it = cache.find(key);
        if (it == cache.end()) return -1;
        if (it->second.second < chrono::system_clock::now()) {
            cache.erase(key);
            // 在list中只删除一个
            lruLists.erase(find(lruLists.begin(), lruLists.end(), key));
            return -1;
        } 
        // 将其放到链表头部
        lruLists.erase(find(lruLists.begin(), lruLists.end(), key));
        lruLists.push_front(key);
        return it->second.first;
    }

    void put(int key, int value, chrono::seconds t) {
        auto it = cache.find(key);
        auto now = chrono::system_clock::now() + t;
        if (it != cache.end()) {
            // 重新设置时间并且放到链表头部
            lruLists.erase(find(lruLists.begin(), lruLists.end(), key));
            lruLists.push_front(key);
            cache[key] = {value, now};
        } else {
            if (lruLists.size() >= capacity) {
                // 删掉最后一个
                int lastKey = lruLists.back();
                lruLists.pop_back();
                cache.erase(lastKey);
            }
            // 添加
            lruLists.push_front(key);
            cache[key] = {value, now};
        }
        return;
    }
    
    uint32_t getCapacity() const {
        return capacity;
    }

    uint32_t size() const {
        return lruLists.size();
    }

private:
    int capacity;
    /// @brief list;
    list<int> lruLists; // only for key
    /// @brief key -> pair<value, time> time_point->时间点 chrono::seconds->一段时间
    unordered_map<int, pair<int, chrono::system_clock::time_point>> cache;
};

int main() {

    LRUCache lru(2);
    lru.put(1, 1, chrono::seconds(5));
    lru.put(2, 2, chrono::seconds(5));

    cout << "lru.get(1) = " << lru.get(1) << endl;
    cout << "lru.get(2) = " << lru.get(2) << endl;
    cout << "cur LRUCache.size() = " << lru.size() << endl;

    this_thread::sleep_for(chrono::seconds(3));

    cout << "sleep 3s later lru.get(1) = " << lru.get(1) << endl;
    cout << "sleep 3s later lru.get(2) = " << lru.get(2) << endl;
    cout << "sleep 3s later cur LRUCache.size() = " << lru.size() << endl;

    this_thread::sleep_for(chrono::seconds(2));

    cout << "sleep 5s later lru.get(1) = " << lru.get(1) << endl;
    cout << "sleep 5s later lru.get(2) = " << lru.get(2) << endl;
    cout << "sleep 5s later cur LRUCache.size() = " << lru.size()
         << endl;

    return 0;
}