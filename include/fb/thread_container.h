#ifndef __THREAD_CONTAINER_H__
#define __THREAD_CONTAINER_H__

#include <boost/asio.hpp>
#include <fb/thread.h>
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
    unique_thread_container _thread_container;
    unique_id_list          _keys;

public:
    std::function<void(void*)> deletor;

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
    template <typename ReturnType, typename T> void enqueue(std::weak_ptr<T>                                           pivot,
                                                            const std::function<bool(fb::thread&)>&                    condition,
                                                            const std::function<async::task<ReturnType>(fb::thread&)>& fn,
                                                            const std::function<void(std::exception&)>&                error,
                                                            const std::function<void(ReturnType&&)>&                   callback)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto thread = pivot->thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        thread->template enqueue<ReturnType>(
            [=, this]() -> async::task<void> {
                auto shared = pivot.lock();
                if (shared == nullptr)
                    throw std::runtime_error("pivot object is expired");

                if (condition(*thread) == false)
                    throw std::runtime_error("condition not satisfied");

                auto active_thread  = shared->thread();
                auto current_thread = this->current();
                if (active_thread != current_thread)
                {
                    this->enqueue(shared.week_from_this(), condition, fn);
                    throw std::runtime_error("active thread not matched");
                }

                co_return co_await fn(*thread);
            },
            error,
            callback);
    }

    template <typename ReturnType, typename T> void enqueue(std::weak_ptr<T>                                           pivot,
                                                            const std::function<bool(fb::thread&)>&                    condition,
                                                            const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        return this->enqueue<ReturnType, T>(
            pivot,
            condition,
            fn,
            [](std::exception& e) {
            },
            [](ReturnType&& value) {
            });
    }

    template <typename ReturnType, typename T> void enqueue(std::weak_ptr<T> pivot, const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        return this->enqueue<ReturnType, T>(
            pivot,
            [](auto& thread) -> bool {
                return true;
            },
            fn,
            [](std::exception& e) {
            },
            [](ReturnType&& value) {
            });
    }

    template <typename T> void enqueue(std::weak_ptr<T>                            pivot,
                                       const std::function<bool(fb::thread&)>&     condition,
                                       const thread::handle_func_type<void>&       fn,
                                       const std::function<void(std::exception&)>& error,
                                       const std::function<void()>&                callback)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto shared = pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto thread = shared->thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        thread->enqueue(
            [=, this](auto& thread) -> async::task<void> {
                auto shared = pivot.lock();
                if (shared == nullptr)
                    throw std::runtime_error("pivot object is expired");

                if (condition(thread) == false)
                    throw std::runtime_error("condition not satisfied");

                auto active_thread = shared->thread();
                if (active_thread != &thread)
                {
                    this->enqueue(pivot, condition, fn);
                }
                else
                {
                    co_await fn(*active_thread);
                }
            },
            error,
            callback);
    }

    template <typename T> void enqueue(std::weak_ptr<T> pivot, const std::function<bool(fb::thread&)>& condition, const thread::handle_func_type<void>& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        return this->enqueue<T>(
            pivot,
            condition,
            fn,
            [](std::exception& e) {
            },
            []() {
            });
    }

    template <typename T> void enqueue(std::weak_ptr<T> pivot, const thread::handle_func_type<void>& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        return this->enqueue<T>(
            pivot,
            [](auto& thread) -> bool {
                return true;
            },
            fn,
            [](std::exception& e) {
            },
            []() {
            });
    }

    template <typename ReturnType, typename T> [[nodiscard]] async::task<ReturnType> dispatch(std::weak_ptr<T>                                           pivot,
                                                                                              const std::function<bool(fb::thread&)>&                    condition,
                                                                                              const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

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

            co_return co_await fn(*target_thread);
        }

        auto promise = std::make_shared<async::task_completion_source<ReturnType>>();
        this->enqueue<ReturnType, T>(
            pivot,
            condition,
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise](ReturnType&& value) {
                promise->set_value(value);
            });
        co_return co_await promise->task();
    }

    template <typename ReturnType, typename T> [[nodiscard]] async::task<ReturnType> dispatch(std::weak_ptr<T> pivot, const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto shared = pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto target_thread = shared->thread();
        if (target_thread == nullptr)
            throw std::runtime_error("no matched thread");

        if (target_thread->id() == std::this_thread::get_id())
        {
            co_return co_await fn(*target_thread);
        }

        auto promise = std::make_shared<async::task_completion_source<ReturnType>>();
        this->enqueue<ReturnType, T>(
            pivot,
            [](auto& thread) -> bool {
                return true;
            },
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise](ReturnType&& value) {
                promise->set_value(value);
            });
        co_return co_await promise->task();
    }

    template <typename T> async::task<void> dispatch(std::weak_ptr<T> pivot, const std::function<bool(fb::thread&)>& condition, const thread::handle_func_type<void>& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

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
            co_return;
        }

        auto promise = std::make_shared<async::task_completion_source<void>>();
        this->enqueue<T>(
            pivot,
            condition,
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise]() {
                promise->set_value();
            });
        co_await promise->task();
    }

    template <typename T> async::task<void> dispatch(std::weak_ptr<T> pivot, const thread::handle_func_type<void>& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto shared = pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto target_thread = shared->thread();
        if (target_thread == nullptr)
            throw std::runtime_error("no matched thread");

        if (target_thread->id() == std::this_thread::get_id())
        {
            co_await fn(*target_thread);
            co_return;
        }

        auto promise = std::make_shared<async::task_completion_source<void>>();
        this->enqueue<T>(
            pivot,
            [](auto& thread) -> bool {
                return true;
            },
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise]() {
                promise->set_value();
            });
        co_await promise->task();
    }

    template <typename T> async::task<void> switching(std::weak_ptr<T> pivot)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        co_await this->dispatch(pivot, [](auto& thread) -> async::task<void> {
            co_return;
        });
    }

    void settimer(const fb::timer::handle_callback_type& fn, const fb::model::timespan& duration);
    void exit();
};

} // namespace fb

#endif // !__THREAD_CONTAINER_H__