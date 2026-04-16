#ifndef __THREAD_CONTAINER_H__
#define __THREAD_CONTAINER_H__

#include <boost/asio.hpp>
#include <coroutine>
#include <fb/thread.h>
#include <fb/timer.h>
#include <fb/thread_switchable.h>
#include <map>

namespace fb {

class server;

class thread_container
{
public:
    using unique_thread           = std::unique_ptr<fb::thread>;
    using unique_thread_container = std::unordered_map<std::thread::id, unique_thread>;
    using unique_id_list          = std::unique_ptr<std::thread::id[]>;

private:
    fb::async_executor&     _executor;
    unique_thread_container _logic_threads;
    unique_id_list          _keys;

public:
    using iterator       = unique_thread_container::iterator;
    using const_iterator = unique_thread_container::const_iterator;

public:
    thread_container(fb::async_executor& executor, uint32_t count);
    thread_container(const thread_container&)             = delete;
    thread_container(thread_container&&)                  = delete;
    thread_container& operator= (thread_container&)       = delete;
    thread_container& operator= (const thread_container&) = delete;
    ~thread_container();

public:
    fb::thread*       at(uint8_t index) const;
    fb::thread*       at(std::thread::id id) const;
    fb::thread*       operator[] (uint8_t index) const;
    fb::thread*       operator[] (std::thread::id id) const;
    fb::thread*       modular(uint32_t id) const;
    fb::thread*       current();
    const fb::thread* current() const;
    uint8_t           count() const;
    size_t            size() const;
    bool              empty() const;
    bool              valid(uint8_t index) const;
    bool              valid(const fb::thread& thread) const;
    bool              valid(thread* thread) const;

public:
    template <typename ReturnType, typename T> void enqueue(std::weak_ptr<T>                   pivot,
                                                            std::function<bool(fb::thread&)>&& condition,
                                                            std::function<async::task<ReturnType>(fb::thread&)>&& fn,
                                                            std::function<void(std::exception&)>&&                error,
                                                            std::function<void(ReturnType&&)>&& callback)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto thread = pivot->thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        thread->template enqueue<ReturnType>(
            [=, this, condition = std::move(condition), fn = std::move(fn)]() -> async::task<void> {
                auto shared = pivot.lock();
                if (shared == nullptr)
                    throw std::runtime_error("pivot object is expired");

                if (condition(*thread) == false)
                    throw std::runtime_error("condition not satisfied");

                auto active_thread  = shared->thread();
                auto current_thread = this->current();
                if (active_thread != current_thread)
                {
                    this->enqueue(shared.week_from_this(), std::move(condition), std::move(fn));
                    throw std::runtime_error("active thread not matched");
                }

                co_return co_await fn(*thread);
            },
            std::move(error),
            std::move(callback));
    }

    template <typename ReturnType, typename T> void enqueue(std::weak_ptr<T>                   pivot,
                                                            std::function<bool(fb::thread&)>&& condition,
                                                            std::function<async::task<ReturnType>(fb::thread&)>&& fn)
    {
        return this->enqueue<ReturnType, T>(
            pivot,
            std::move(condition),
            std::move(fn),
            [](std::exception& e) {
            },
            [](ReturnType&& value) {
            });
    }

    template <typename ReturnType, typename T> void enqueue(std::weak_ptr<T>                                      pivot,
                                                            std::function<async::task<ReturnType>(fb::thread&)>&& fn)
    {
        return this->enqueue<ReturnType, T>(
            pivot,
            [](auto& thread) -> bool {
                return true;
            },
            std::move(fn),
            [](std::exception& e) {
            },
            [](ReturnType&& value) {
            });
    }

    template <typename T> void enqueue(std::weak_ptr<T>                       pivot,
                                       std::function<bool(fb::thread&)>&&     condition,
                                       thread::handle_func_type<void>&&       fn,
                                       std::function<void(std::exception&)>&& error,
                                       std::function<void()>&&                callback)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto shared = pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto thread = shared->thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        thread->enqueue(
            [=, this, condition = std::move(condition), fn = std::move(fn)](auto& thread) mutable -> async::task<void> {
                auto shared = pivot.lock();
                if (shared == nullptr)
                    throw std::runtime_error("pivot object is expired");

                if (condition(thread) == false)
                    throw std::runtime_error("condition not satisfied");

                auto active_thread = shared->thread();
                if (active_thread != &thread)
                {
                    this->enqueue(pivot, std::move(condition), std::move(fn));
                }
                else
                {
                    co_await fn(*active_thread);
                }
            },
            std::move(error),
            std::move(callback));
    }

    template <typename T> void enqueue(std::weak_ptr<T>                   pivot,
                                       std::function<bool(fb::thread&)>&& condition,
                                       thread::handle_func_type<void>&&   fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        return this->enqueue<T>(
            pivot,
            std::move(condition),
            std::move(fn),
            [](std::exception& e) {
            },
            []() {
            });
    }

    template <typename T> void enqueue(std::weak_ptr<T> pivot, thread::handle_func_type<void>&& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        return this->enqueue<T>(
            pivot,
            [](auto& thread) -> bool {
                return true;
            },
            std::move(fn),
            [](std::exception& e) {
            },
            []() {
            });
    }

    template <typename ReturnType, typename T> [[nodiscard]] async::task<ReturnType>
    dispatch(std::weak_ptr<T>                                      pivot,
             std::function<bool(fb::thread&)>&&                    condition,
             std::function<async::task<ReturnType>(fb::thread&)>&& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto promise    = std::make_shared<async::task_completion_source<ReturnType>>();
        auto need_yield = false;
        try
        {
            auto shared = pivot.lock();
            if (shared == nullptr)
                throw std::runtime_error("pivot object is expired");

            auto target_thread = shared->thread();
            if (target_thread == nullptr)
                throw std::runtime_error("no matched thread");

            if (target_thread->id() == std::this_thread::get_id())
            {
                if (condition(*target_thread) == false)
                    throw std::runtime_error("condition not satisfied");

                auto value = co_await fn(*target_thread);
                promise->set_value(std::move(value));
                co_return co_await promise->task();
            }

            this->enqueue<ReturnType, T>(
                pivot,
                std::move(condition),
                std::move(fn),
                [promise](std::exception& e) {
                    promise->set_exception(std::make_exception_ptr(e));
                },
                [promise](ReturnType&& value) {
                    promise->set_value(value);
                });
            co_return co_await promise->task();
        }
        catch (...)
        {
            promise->set_exception(std::make_exception_ptr(std::current_exception()));
            need_yield = true;
        }

        if (need_yield)
            co_await std::suspend_always{};
        co_return co_await promise->task();
    }

    template <typename ReturnType, typename T> [[nodiscard]] async::task<ReturnType>
    dispatch(std::weak_ptr<T> pivot, std::function<async::task<ReturnType>(fb::thread&)>&& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto promise    = std::make_shared<async::task_completion_source<ReturnType>>();
        auto need_yield = false;
        try
        {
            auto shared = pivot.lock();
            if (shared == nullptr)
                throw std::runtime_error("pivot object is expired");

            auto target_thread = shared->thread();
            if (target_thread == nullptr)
                throw std::runtime_error("no matched thread");

            if (target_thread->id() == std::this_thread::get_id())
            {
                auto value = co_await fn(*target_thread);
                promise->set_value(std::move(value));
                co_return co_await promise->task();
            }

            this->enqueue<ReturnType, T>(
                pivot,
                [](auto& thread) -> bool {
                    return true;
                },
                std::move(fn),
                [promise](std::exception& e) {
                    promise->set_exception(std::make_exception_ptr(e));
                },
                [promise](ReturnType&& value) {
                    promise->set_value(value);
                });
            co_return co_await promise->task();
        }
        catch (...)
        {
            promise->set_exception(std::make_exception_ptr(std::current_exception()));
            need_yield = true;
        }

        if (need_yield)
            co_await std::suspend_always{};
        co_return co_await promise->task();
    }

    template <typename T> async::task<void> dispatch(std::weak_ptr<T>                   pivot,
                                                     std::function<bool(fb::thread&)>&& condition,
                                                     thread::handle_func_type<void>&&   fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto promise    = std::make_shared<async::task_completion_source<void>>();
        auto need_yield = false;
        try
        {
            auto shared = pivot.lock();
            if (shared == nullptr)
                throw std::runtime_error("pivot object is expired");

            auto target_thread = shared->thread();
            if (target_thread == nullptr)
                throw std::runtime_error("no matched thread");

            if (target_thread->id() == std::this_thread::get_id())
            {
                if (condition(*target_thread) == false)
                    throw std::runtime_error("condition not satisfied");

                co_await fn(*target_thread);
                promise->set_value();
                co_return co_await promise->task();
            }

            this->enqueue<T>(
                pivot,
                std::move(condition),
                std::move(fn),
                [promise](std::exception& e) {
                    promise->set_exception(std::make_exception_ptr(e));
                },
                [promise]() {
                    promise->set_value();
                });
            co_return co_await promise->task();
        }
        catch (...)
        {
            promise->set_exception(std::make_exception_ptr(std::current_exception()));
            need_yield = true;
        }

        if (need_yield)
            co_await std::suspend_always{};
        co_return co_await promise->task();
    }

    template <typename T> async::task<void> dispatch(std::weak_ptr<T> pivot, thread::handle_func_type<void>&& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto promise    = std::make_shared<async::task_completion_source<void>>();
        auto need_yield = false;
        try
        {
            auto shared = pivot.lock();
            if (shared == nullptr)
                throw std::runtime_error("pivot object is expired");

            auto target_thread = shared->thread();
            if (target_thread == nullptr)
                throw std::runtime_error("no matched thread");

            if (target_thread->id() == std::this_thread::get_id())
            {
                co_await fn(*target_thread);
                promise->set_value();
                co_return co_await promise->task();
            }

            this->enqueue<T>(
                pivot,
                [](auto& thread) -> bool {
                    return true;
                },
                std::move(fn),
                [promise](std::exception& e) {
                    promise->set_exception(std::make_exception_ptr(e));
                },
                [promise]() {
                    promise->set_value();
                });
            co_return co_await promise->task();
        }
        catch (...)
        {
            promise->set_exception(std::make_exception_ptr(std::current_exception()));
            need_yield = true;
        }

        if (need_yield)
            co_await std::suspend_always{};
        co_return co_await promise->task();
    }

    template <typename T> async::task<void> switching(std::weak_ptr<T> pivot)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        co_await this->dispatch(pivot, [](auto& thread) -> async::task<void> {
            co_return;
        });
    }

    void        settimer(std::function<async::task<void>(const fb::model::datetime&, std::thread::id)>&& fn,
                         const fb::model::timespan& duration);
    void        exit();
    fb::thread* least_loaded() const;

public:
    iterator       begin();
    iterator       end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
};

} // namespace fb

#endif // !__THREAD_CONTAINER_H__