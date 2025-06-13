#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <shared_mutex>
#include <functional>

namespace fb {

template <typename ValueType>
class locker
{
private:
    mutable std::shared_mutex _mutex;
    ValueType                 _value;

public:
    template <typename... Args>
    locker(Args&&... args) :
        _value(std::forward<Args>(args)...)
    { }

    locker(const locker&) = delete;
    locker(locker&&)      = delete;
    ~locker()             = default;

    template <typename ReturnType>
    ReturnType write(const std::function<ReturnType(ValueType&)>& fn)
    {
        auto _ = std::unique_lock(this->_mutex);
        return fn(this->_value);
    }

    void write(const std::function<void(ValueType&)>& fn)
    {
        auto _ = std::unique_lock(this->_mutex);
        fn(this->_value);
    }

    template <typename ReturnType>
    ReturnType read(const std::function<ReturnType(const ValueType&)>& fn) const
    {
        auto _ = std::shared_lock(this->_mutex);
        return fn(this->_value);
    }

    void read(const std::function<void(const ValueType&)>& fn) const
    {
        auto _ = std::shared_lock(this->_mutex);
        fn(this->_value);
    }
};

} // namespace fb

#endif
