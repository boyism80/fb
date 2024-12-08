#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <mutex>
#include <functional>
#include <shared_mutex>
#include <async/task.h>
#include <fb/abstract.h>
#include <fb/logger.h>

namespace fb {

template <typename ValueType>
class locker
{
private:
    std::shared_mutex _mutex;
    ValueType         _value;
    fb::context&      _context;

public:
    template <typename... Args>
    locker(fb::context& context, Args&&... args) :
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
        auto _ = std::unique_lock(this->_mutex);

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
    ReturnType read(const std::function<ReturnType(const ValueType&)>& fn)
    {
        auto _ = std::shared_lock(this->_mutex);

        return fn(this->_value);
    }

    template <typename ReturnType>
    [[nodiscard]] async::task<ReturnType> lock(const std::function<async::task<ReturnType>(ValueType&)>& fn)
    {
        auto _ = std::unique_lock(this->_mutex);

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
        }
        else
        {
            try
            {
                co_return co_await fn(this->_value);
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
                throw e;
            }
        }
    }

    template <typename ReturnType>
    [[nodiscard]] async::task<ReturnType> read(const std::function<async::task<ReturnType>(const ValueType&)>& fn)
    {
        auto _ = std::shared_lock(this->_mutex);

        try
        {
            co_return co_await fn(this->_value);
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
            throw e;
        }
    }
};

} // namespace fb

#endif