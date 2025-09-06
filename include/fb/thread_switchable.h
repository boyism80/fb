#ifndef __THREAD_SWITCHABLE_H__
#define __THREAD_SWITCHABLE_H__

#include <stdexcept>
#include <memory>
#include <fb/lua.h>

namespace fb {

class thread;

class thread_switchable : public lua::luable
{
public:
    LUA_PROTOTYPE

private:
    using super = lua::luable;

protected:
    thread_switchable() = default;
    thread_switchable(uint32_t id);

public:
    virtual ~thread_switchable() = default;

public:
    virtual fb::thread* thread() const = 0;
    virtual void        assert_thread() const;
    virtual bool        matched_thread() const;

    std::weak_ptr<thread_switchable> weak_from_this()
    {
        return std::weak_ptr<thread_switchable>(this->shared_from_this_as<thread_switchable>());
    }

    std::weak_ptr<const thread_switchable> weak_from_this() const
    {
        return std::weak_ptr<const thread_switchable>(this->shared_from_this_as<thread_switchable>());
    }

    template <typename T>
    std::weak_ptr<T> weak_from_this_as()
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must inherit from thread_switchable");
        return std::static_pointer_cast<T>(this->shared_from_this());
    }

    template <typename T>
    std::weak_ptr<const T> weak_from_this_as() const
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must inherit from thread_switchable");
        return std::static_pointer_cast<const T>(this->shared_from_this());
    }

    template <typename T>
    std::shared_ptr<T> shared_from_this_as()
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must inherit from thread_switchable");
        return std::static_pointer_cast<T>(this->shared_from_this());
    }

    template <typename T>
    std::shared_ptr<const T> shared_from_this_as() const
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must inherit from thread_switchable");
        return std::static_pointer_cast<const T>(this->shared_from_this());
    }
};

} // namespace fb

#endif