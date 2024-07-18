#ifndef __PQUEUE_H__
#define __PQUEUE_H__

#include <queue>
#include <mutex>
#include <optional>

namespace fb {

template <typename T>
using pqnode = std::pair<uint32_t, T>;

template <typename T>
class queue_comparison
{
public:
    bool operator() (pqnode<T>& val1, pqnode<T>& val2)
    {
        return val1.first < val2.first;
    }
};

template <typename T>
using pqueue = std::priority_queue<pqnode<T>, std::deque<pqnode<T>>, queue_comparison<T>>;

template <typename T>
class queue : private pqueue<T>
{
private:
    std::recursive_mutex                            _mutex;

public:
    queue() = default;
    ~queue() = default;

    queue(const queue&) = delete;
    queue(queue&&) = delete;

    queue& operator = (queue&) = delete;
    queue& operator = (const queue&) = delete;

public:
    bool                                            empty()
    {
        auto _ = std::lock_guard(this->_mutex);
        return pqueue::empty();
    }
    void                                            enqueue(T&& ref, uint32_t priority = 0)
    {
        auto _ = std::lock_guard(this->_mutex);
        pqueue<T>::push({priority, std::move(ref)});
    }

    bool                                dequeue(const std::function<void(T&&)>& fn)
    {
        auto _ = std::lock_guard(this->_mutex);
        auto empty = pqueue<T>::empty();
        if (empty)
            return false;

        auto& top = const_cast<T&>(this->top().second);
        auto x = std::move(top);
        fn(std::move(x));
        this->pop();
        return true;
    }
};

}

#endif