#ifndef __ASYNC_LOCAL_H__
#define __ASYNC_LOCAL_H__

#include <fb/execution_context.h>
#include <atomic>
#include <utility>

namespace fb {

namespace detail {

inline std::size_t allocate_async_local_slot()
{
    static std::atomic<std::size_t> next{1};
    return next.fetch_add(1, std::memory_order_relaxed);
}

} // namespace detail

template <typename T>
class async_local
{
public:
    [[nodiscard]] static std::size_t slot_id()
    {
        static std::size_t id = detail::allocate_async_local_slot();
        return id;
    }

    [[nodiscard]] static T* try_get()
    {
        auto ambient = execution_context::current();
        if (ambient == nullptr)
            return nullptr;

        return ambient->slot<T>(slot_id());
    }
};

} // namespace fb

#endif // !__ASYNC_LOCAL_H__
