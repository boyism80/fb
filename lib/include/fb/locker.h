#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <mutex>
#include <functional>
#include <shared_mutex>
#include <fb/abstract.h>
#include <fb/logger.h>

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
        auto _ = std::unique_lock(this->_mutex);

        return fn(this->_value);
    }

    void lock(const std::function<void(ValueType&)>& fn)
    {
        auto _ = std::unique_lock(this->_mutex);

        fn(this->_value);
    }
};

} // namespace fb

#endif