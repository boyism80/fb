#ifndef __COROUTINE_HPP__
#define __COROUTINE_HPP__

#include "coroutine.h"
#include <stdio.h>

template <typename T, typename SUSPEND_TYPE>
fb::base_task<T, SUSPEND_TYPE>::base_task(handle_type<SUSPEND_TYPE> handler) : handler(handler)
{ }

template <typename T, typename SUSPEND_TYPE>
fb::base_task<T, SUSPEND_TYPE>::base_task(base_task&& r) noexcept : handler(r.handler)
{
    r.handler = nullptr;
}

template <typename T, typename SUSPEND_TYPE>
fb::base_task<T, SUSPEND_TYPE>::~base_task()
{
    if (this->handler)
        this->handler.destroy();
}

template <typename T, typename SUSPEND_TYPE>
bool fb::base_task<T, SUSPEND_TYPE>::await_ready()
{
    return false;
}

template <typename T, typename SUSPEND_TYPE>
void fb::base_task<T, SUSPEND_TYPE>::await_suspend(std::coroutine_handle<> raw)
{
    auto& promise = this->handler.promise();
    if (promise.error)
        std::rethrow_exception(promise.error);

    if (this->handler.done())
        raw.resume();
    else
        promise.parent = raw;
}

template <typename T, typename SUSPEND_TYPE>
bool fb::base_task<T, SUSPEND_TYPE>::done()
{
    if (!this->handler)
        throw std::runtime_error("destroyed task");

    return this->handler.done();
}

template <typename T, typename SUSPEND_TYPE>
void fb::base_task<T, SUSPEND_TYPE>::operator = (base_task&& r) noexcept
{
    this->handler = r.handler;
    r.handler = nullptr;
}

template <typename T, typename SUSPEND_TYPE>
fb::task<T, SUSPEND_TYPE>::task(handle_type<SUSPEND_TYPE> handler) : base_task<T, SUSPEND_TYPE>(handler)
{}

template <typename T, typename SUSPEND_TYPE>
fb::task<T, SUSPEND_TYPE>::task(task&& r) : base_task<T, SUSPEND_TYPE>(static_cast<base_task<T, SUSPEND_TYPE>&&>(r))
{}


template <typename T, typename SUSPEND_TYPE>
T& fb::task<T, SUSPEND_TYPE>::value()
{
    auto& promise = static_cast<promise_type&>(this->handler.promise());
    if (promise.error)
        std::rethrow_exception(promise.error);

    if (promise.value.has_value() == false)
        throw std::runtime_error("value is empty");

    if constexpr (std::is_class_v<T>)
    {
        return promise.value.value().get();
    }
    else
    {
        return promise.value.value();
    }
}

template <typename T, typename SUSPEND_TYPE>
T& fb::task<T, SUSPEND_TYPE>::await_resume()
{
    return this->value();
}

template <typename T, typename SUSPEND_TYPE>
T& fb::task<T, SUSPEND_TYPE>::wait()
{
    constexpr auto term = 100ms;
    while (!this->done())
    {
        std::this_thread::sleep_for(term);
    }

    return this->value();
}

template <typename T, typename SUSPEND_TYPE>
void fb::task<T, SUSPEND_TYPE>::operator = (task&& r)
{
    base_task<T, SUSPEND_TYPE>::operator = (static_cast<base_task<T, SUSPEND_TYPE>&&>(r));
}



template <typename T, typename SUSPEND_TYPE>
fb::task<T, SUSPEND_TYPE> fb::task<T, SUSPEND_TYPE>::promise_type::get_return_object()
{
    return task<T, SUSPEND_TYPE>(handle_type<SUSPEND_TYPE>::from_promise(*this));
}

template <typename T, typename SUSPEND_TYPE>
template<std::convertible_to<T> From>
void fb::task<T, SUSPEND_TYPE>::promise_type::return_value(From&& from)
{
    this->value = std::forward<From>(from);
}

template <typename SUSPEND_TYPE>
fb::task<void, SUSPEND_TYPE>::task(handle_type<SUSPEND_TYPE> handler) : base_task<void, SUSPEND_TYPE>(handler)
{}

template <typename SUSPEND_TYPE>
fb::task<void, SUSPEND_TYPE>::task(task&& r) noexcept : base_task<void, SUSPEND_TYPE>(static_cast<base_task<void, SUSPEND_TYPE>&&>(r))
{}

template <typename SUSPEND_TYPE>
void fb::task<void, SUSPEND_TYPE>::await_resume()
{
}

template <typename SUSPEND_TYPE>
void fb::task<void, SUSPEND_TYPE>::wait()
{
    constexpr auto term = 100ms;
    while (!this->done())
    {
        std::this_thread::sleep_for(term);
    }
}

template <typename SUSPEND_TYPE>
void fb::task<void, SUSPEND_TYPE>::operator = (task&& r) noexcept
{
    base_task<void, SUSPEND_TYPE>::operator = (static_cast<base_task<void, SUSPEND_TYPE>&&>(r));
}


template <typename SUSPEND_TYPE>
fb::task<void, SUSPEND_TYPE> fb::task<void, SUSPEND_TYPE>::promise_type::get_return_object()
{
    return task<void, SUSPEND_TYPE>(handle_type< SUSPEND_TYPE>::from_promise(*this));
}

template <typename SUSPEND_TYPE>
void fb::task<void, SUSPEND_TYPE>::promise_type::return_void()
{ }

template <typename T>
fb::base_awaiter<T>::base_awaiter(fb::task<T, std::suspend_always>&& task) : task(std::move(task))
{}

template <typename T>
fb::base_awaiter<T>::base_awaiter(base_awaiter&& r) noexcept : error(r.error), task(std::move(r.task))
{
    r.error = nullptr;
}

template <typename T>
void fb::base_awaiter<T>::set_error(const std::exception& e)
{
    this->error = std::make_exception_ptr(e);
    this->task.handler.resume();
}


template <typename T>
fb::awaiter<T>::awaiter() : base_awaiter<T>(handler())
{}

template <typename T>
fb::awaiter<T>::awaiter(awaiter&& r) noexcept : base_awaiter<T>(static_cast<base_awaiter<T>&&>(r)), result(std::move(r.result))
{}

template <typename T>
fb::task<T, std::suspend_always> fb::awaiter<T>::handler()
{
    if (this->error != nullptr)
        std::rethrow_exception(this->error);

    if (this->result.has_value() == false)
        throw std::runtime_error("has no value");

    co_return result.value();
}

template <typename T>
template <typename Q = T>
typename std::enable_if<std::is_class<Q>::value>::type fb::awaiter<T>::set_result(T&& value)
{
    this->result = value;
    this->task.handler.resume();
}

template <typename T>
template <typename Q = T>
typename std::enable_if<std::is_class<Q>::value>::type fb::awaiter<T>::set_result(T& value)
{
    this->result = value;
    this->task.handler.resume();
}

template <typename T>
template <typename Q = T>
typename std::enable_if<!std::is_class<Q>::value>::type fb::awaiter<T>::set_result(T value)
{
    this->result = value;
    this->task.handler.resume();
}

#endif