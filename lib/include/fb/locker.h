#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <mutex>
#include <functional>
#include <async/task.h>

namespace fb {

template <typename ValueType>
class locker
{
private:
    std::recursive_mutex _mutex;
    ValueType            _value;

public:
    template <typename... Args>
    locker(Args&&... args) :
        _value(std::forward<Args>(args)...)
    { }
    locker(const locker&) = delete;
    locker(locker&&)      = delete;
    ~locker()             = default;

public:
    template <typename ReturnType>
    ReturnType lock(const std::function<ReturnType(ValueType&)>& fn)
    {
        auto _ = std::lock_guard(this->_mutex);
        if constexpr (std::is_same_v<ReturnType, void>)
        {
            fn(this->_value);
        }
        else
        {
            return fn(this->_value);
        }
    }

    template <typename ReturnType>
    async::task<ReturnType> lock(const std::function<async::task<ReturnType>(ValueType&)>& fn)
    {
        auto _ = std::lock_guard(this->_mutex);
        if constexpr (std::is_same_v<ReturnType, void>)
        {
            co_await fn(this->_value);
        }
        else
        {
            co_return co_await fn(this->_value);
        }
    }
};

} // namespace fb

#endif