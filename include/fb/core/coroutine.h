#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include "promise.h"
#include <optional>

namespace fb {

    template <typename SUSPEND_TYPE = std::suspend_never>
    using handle_type = std::coroutine_handle<base_promise<SUSPEND_TYPE>>;

    template <typename T, typename T2 = void> struct task_result;
    template <typename T> struct task_result<T, typename std::enable_if<std::is_class<T>::value>::type> { typedef std::optional<std::reference_wrapper<T>> type; };
    template <typename T> struct task_result<T, typename std::enable_if<!std::is_class<T>::value>::type> { typedef std::optional<T> type; };

    template <typename T, typename SUSPEND_TYPE>
    class base_task
    {
    public:
        handle_type<SUSPEND_TYPE>               handler;

    public:
        base_task(handle_type<SUSPEND_TYPE> handler);
        base_task(const base_task&) = delete;
        base_task(base_task&& r) noexcept;
        ~base_task();

        bool                                    await_ready();
        void                                    await_suspend(std::coroutine_handle<> raw);
        bool                                    done();
        void                                    operator = (const base_task&) = delete;
        void                                    operator = (base_task&& r) noexcept;
    };

    template <typename T, typename SUSPEND_TYPE = std::suspend_never>
    class task : public base_task<T, SUSPEND_TYPE>
    {
    public:
        class promise_type;

    public:
        task(handle_type<SUSPEND_TYPE> handler);
        task(const task&) = delete;
        task(task&& r);

    public:
        T& value();
        T& await_resume();
        T& wait();
        void                                    operator = (const task&) = delete;
        void                                    operator = (task&& r);
    };


    template <typename T, typename SUSPEND_TYPE>
    class task<T, SUSPEND_TYPE>::promise_type : public base_promise<SUSPEND_TYPE>
    {
    public:
        task_result<T>::type                    value;

    public:
        task<T, SUSPEND_TYPE>                   get_return_object();

        template<std::convertible_to<T> From>
        void                                    return_value(From&& from);
    };


    template <typename SUSPEND_TYPE>
    class task<void, SUSPEND_TYPE> : public base_task<void, SUSPEND_TYPE>
    {
    public:
        class promise_type;

    public:
        task(handle_type<SUSPEND_TYPE> handler);
        task(const task&) = delete;
        task(task&& r) noexcept;

    public:
        void                                    await_resume();
        void                                    wait();
        void                                    operator = (const task&) = delete;
        void                                    operator = (task&& r) noexcept;
    };

    template <typename SUSPEND_TYPE>
    class task<void, SUSPEND_TYPE>::promise_type : public base_promise< SUSPEND_TYPE>
    {
    public:
        task<void, SUSPEND_TYPE>                get_return_object();
        void                                    return_void();
    };


    template <typename T>
    class base_awaiter
    {
    protected:
        std::exception_ptr                      error;

    public:
        fb::task<T, std::suspend_always>        task;

    protected:
        base_awaiter(fb::task<T, std::suspend_always>&& task);
        base_awaiter(const base_awaiter&) = delete;
        base_awaiter(base_awaiter&& r) noexcept;
        ~base_awaiter() = default;

    public:
        void                                    set_error(const std::exception& e);
    };

    template <typename T>
    class awaiter : public base_awaiter<T>
    {
    private:
        fb::task_result<T>::type                result;

    public:
        awaiter();
        awaiter(const awaiter&) = delete;
        awaiter(awaiter&& r) noexcept;
        ~awaiter() = default;

    private:
        fb::task<T, std::suspend_always>        handler();

    public:
        template <typename Q = T>
        typename std::enable_if<std::is_class<Q>::value>::type set_result(T&& value);

        template <typename Q = T>
        typename std::enable_if<std::is_class<Q>::value>::type set_result(T& value);

        template <typename Q = T>
        typename std::enable_if<!std::is_class<Q>::value>::type set_result(T value);
    };


    template <>
    class awaiter<void> : public base_awaiter<void>
    {
    public:
        awaiter() : base_awaiter<void>(handler()) {}
        awaiter(const awaiter&) = delete;
        awaiter(awaiter&& r) noexcept : base_awaiter<void>(static_cast<base_awaiter<void>&&>(r)) {}
        ~awaiter() = default;

    private:
        fb::task<void, std::suspend_always>     handler()
        {
            if (this->error != nullptr)
                std::rethrow_exception(this->error);

            co_return;
        }

    public:
        void                                    resume()
        {
            this->task.handler.resume();
            this->task.handler.destroy();
            this->task.handler = nullptr;
        }
    };

}

#include "coroutine.hpp"
#include "generator.h"

#endif