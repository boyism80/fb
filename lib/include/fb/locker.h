#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <mutex>
#include <functional>
#include <async/task.h>
#include <fb/abstract.h>
#include <fb/logger.h>

namespace fb {

template <typename ValueType>
class locker
{
private:
    std::recursive_mutex _mutex;
    ValueType            _value;
    fb::icontext&        _context;
    bool                 _locked = false;

public:
    template <typename... Args>
    locker(fb::icontext& context, Args&&... args) :
        _context(context),
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

        this->_locked = true;
        if constexpr (std::is_same_v<ReturnType, void>)
        {
            fn(this->_value);
            this->_locked = false;
        }
        else
        {
            auto result   = fn(this->_value);
            this->_locked = false;
            return result;
        }
    }

    template <typename ReturnType>
    async::task<ReturnType> lock(const std::function<async::task<ReturnType>(ValueType&)>& fn)
    {
        auto _ = std::lock_guard(this->_mutex);

        auto thread = this->_context.current_thread();
        if (thread == nullptr)
            throw std::runtime_error("cannot find current thread in locker");

        while (this->_locked)
        {
            co_await thread->sleep(100ms);
        }

        this->_locked = true;
        if constexpr (std::is_same_v<ReturnType, void>)
        {
            try
            {
                co_await fn(this->_value);
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
                throw e;
            }
            this->_locked = false;
        }
        else
        {
            try
            {
                auto result   = co_await fn(this->_value);
                this->_locked = false;
                co_return result;
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
                this->_locked = false;
                throw e;
            }
        }
    }
};

} // namespace fb

#endif