#ifndef __THREAD_CONTAINER_H__
#define __THREAD_CONTAINER_H__

#include <async/propagation.h>
#include <fb/execution_context.h>
#include <fb/thread.h>
#include <fb/thread_switchable.h>
#include <fb/timer.h>

#include <boost/asio.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace fb {

class server;

class thread_container
{
public:
    using unique_thread           = std::unique_ptr<fb::thread>;
    using unique_thread_container = std::unordered_map<std::thread::id, unique_thread>;
    using unique_id_list          = std::unique_ptr<std::thread::id[]>;
    using iterator                = unique_thread_container::iterator;
    using const_iterator          = unique_thread_container::const_iterator;

    template <typename T, typename PivotT>
    class builder;

private:
    fb::async_executor&     _executor;
    unique_thread_container _logic_threads;
    unique_id_list          _keys;

public:
    thread_container(fb::async_executor& executor, uint32_t count);
    thread_container(const thread_container&)             = delete;
    thread_container(thread_container&&)                  = delete;
    thread_container& operator= (thread_container&)       = delete;
    thread_container& operator= (const thread_container&) = delete;
    ~thread_container();

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

    template <typename T = void, typename PivotT> [[nodiscard]] builder<T, PivotT>
    new_builder(std::weak_ptr<PivotT> pivot)
    {
        static_assert(std::is_base_of_v<thread_switchable, PivotT>, "PivotT must be a thread_switchable");

        return builder<T, PivotT>(*this, std::move(pivot));
    }

    template <typename PivotT> [[nodiscard]] async::task<void> switching(std::weak_ptr<PivotT> pivot)
    {
        static_assert(std::is_base_of_v<thread_switchable, PivotT>, "PivotT must be a thread_switchable");

        auto builder = this->template new_builder<void, PivotT>(std::move(pivot));
        builder.func = [](auto&) -> async::task<void> {
            co_return;
        };
        builder.context = execution_context::token();
        co_return co_await builder.dispatch();
    }

    void        settimer(std::function<async::task<void>(const fb::model::datetime&, std::thread::id)>&& fn,
                         const fb::model::timespan&                                                      duration);
    void        exit();
    void        join();
    fb::thread* least_loaded() const;

    iterator       begin();
    iterator       end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

private:
    template <typename PivotT>
    fb::thread* thread_for(const std::weak_ptr<PivotT>& pivot) const
    {
        static_assert(std::is_base_of_v<thread_switchable, PivotT>, "PivotT must be a thread_switchable");

        auto shared = pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto thread = shared->thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        return thread;
    }
};

template <typename T, typename PivotT>
class thread_container::builder
{
    friend class thread_container;

private:
    // A pivot that keeps changing threads would otherwise chain hops forever.
    static constexpr size_t MAX_PIVOT_HOPS = 8;

    thread_container&     _container;
    std::weak_ptr<PivotT> _pivot;
    size_t                _hops = 0;

public:
    std::function<bool(fb::thread&)> when;
    thread::handle_func_type<T>      func;
    thread::handle_error_type        on_error;
    std::function<void()>            on_complete;
    size_t                           retry_count = 0;
    async::propagation::token        context;

    void enqueue()
    {
        if (this->func == nullptr)
            throw std::runtime_error("thread container builder: func is not set");

        auto* target      = this->_container.thread_for<PivotT>(this->_pivot);
        auto  fn_holder   = std::make_shared<thread::handle_func_type<T>>(std::move(this->func));
        auto  when_fn     = this->when ? this->when : std::function<bool(fb::thread&)>([](auto&) {
            return true;
        });
        auto  on_error    = thread::on_error(std::move(this->on_error));
        auto  on_complete = std::move(this->on_complete);
        // Reflects the latest attempt: true when func ran here, false when it hopped to the pivot's new thread.
        auto ran = std::make_shared<bool>(false);

        // The result of an enqueued func is discarded, so the inner task is void for every T.
        // Exceptions from func are not caught here so retry_exception still reaches the retry loop.
        auto inner = target->template new_builder<void>();
        inner.func = [container   = &this->_container,
                      pivot       = this->_pivot,
                      hops        = this->_hops,
                      retry_count = this->retry_count,
                      context     = this->context,
                      fn_holder,
                      when_fn,
                      on_error,
                      on_complete,
                      ran](fb::thread& thread) -> async::task<void> {
            auto shared = pivot.lock();
            if (shared == nullptr)
                throw std::runtime_error("pivot object is expired");

            if (when_fn(thread) == false)
                throw std::runtime_error("condition not satisfied");

            if (shared->thread() != &thread)
            {
                if (hops >= MAX_PIVOT_HOPS)
                    throw std::runtime_error("pivot moved too many times");

                auto retry  = container->template new_builder<T, PivotT>(shared->template weak_from_this_as<PivotT>());
                retry._hops = hops + 1;
                retry.when  = when_fn;
                retry.func  = *fn_holder;
                retry.on_error    = on_error;
                retry.on_complete = on_complete;
                retry.retry_count = retry_count;
                retry.context     = context;
                retry.enqueue();

                // A retried attempt may have run func earlier; only the builder that runs it last reports completion.
                *ran = false;
            }
            else
            {
                *ran = true;
                if constexpr (std::is_same_v<T, void>)
                    co_await (*fn_holder)(thread);
                else
                    std::ignore = co_await (*fn_holder)(thread);
            }
        };
        inner.retry_count = this->retry_count;
        inner.context     = this->context;
        inner.on_error    = on_error;
        inner.on_complete = [ran, on_complete]() {
            if (*ran && on_complete)
                on_complete();
        };
        inner.enqueue();
    }

    [[nodiscard]] async::task<T> dispatch()
    {
        if (this->func == nullptr)
            throw std::runtime_error("thread container builder: func is not set");

        auto shared = this->_pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto* target_thread = shared->thread();
        if (target_thread == nullptr)
            throw std::runtime_error("no matched thread");

        auto when_fn = this->when ? this->when : std::function<bool(fb::thread&)>([](auto&) {
            return true;
        });

        if (this->retry_count == 0 && target_thread->id() == std::this_thread::get_id())
        {
            // Same-thread fast path. Do not use suspend_always on errors:
            // nothing resumes it, so the caller coroutine would hang forever
            // with no exception propagation (and no logs).
            if (when_fn(*target_thread) == false)
                throw std::runtime_error("condition not satisfied");

            execution_context::pending(this->context);

            if constexpr (std::is_same_v<T, void>)
            {
                co_await this->func(*target_thread);
                co_return;
            }
            else
            {
                co_return co_await this->func(*target_thread);
            }
        }

        auto fn_holder = std::make_shared<thread::handle_func_type<T>>(std::move(this->func));
        auto inner     = target_thread->template new_builder<T>();
        inner.func     = [container   = &this->_container,
                      pivot       = this->_pivot,
                      hops        = this->_hops,
                      retry_count = this->retry_count,
                      context     = this->context,
                      fn_holder,
                      when_fn](fb::thread& thread) -> async::task<T> {
            auto shared = pivot.lock();
            if (shared == nullptr)
                throw std::runtime_error("pivot object is expired");

            if (when_fn(thread) == false)
                throw std::runtime_error("condition not satisfied");

            if (shared->thread() != &thread)
            {
                if (hops >= MAX_PIVOT_HOPS)
                    throw std::runtime_error("pivot moved too many times");

                auto retry  = container->template new_builder<T, PivotT>(shared->template weak_from_this_as<PivotT>());
                retry._hops = hops + 1;
                retry.when  = when_fn;
                retry.func  = *fn_holder;
                retry.retry_count = retry_count;
                retry.context     = context;
                if constexpr (std::is_same_v<T, void>)
                {
                    co_await retry.dispatch();
                    co_return;
                }
                else
                {
                    co_return co_await retry.dispatch();
                }
            }
            else
            {
                if constexpr (std::is_same_v<T, void>)
                {
                    co_await (*fn_holder)(thread);
                    co_return;
                }
                else
                {
                    co_return co_await (*fn_holder)(thread);
                }
            }
        };
        inner.retry_count = this->retry_count;
        inner.context     = this->context;
        co_return co_await inner.dispatch();
    }

private:
    explicit builder(thread_container& container, std::weak_ptr<PivotT> pivot) :
        _container(container),
        _pivot(std::move(pivot))
    { }
};

} // namespace fb

#endif // !__THREAD_CONTAINER_H__
