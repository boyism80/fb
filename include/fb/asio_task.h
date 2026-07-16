#ifndef __FB_ASIO_TASK_H__
#define __FB_ASIO_TASK_H__

#include <async/awaitable_then.h>
#include <async/task.h>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/post.hpp>
#include <exception>
#include <optional>
#include <type_traits>
#include <utility>

namespace fb {

/**
 * @brief Bridge async::task into a Boost.Asio async operation.
 *
 * Allows co_await of cpp-async tasks inside boost::asio::awaitable without
 * blocking the IO thread (unlike async::awaitable_get).
 *
 * Completion signature: void(std::exception_ptr)
 * With use_awaitable, exceptions are rethrown at the co_await site.
 */
template <typename CompletionToken>
auto async_await_task(async::task<void> task, CompletionToken&& token)
{
    return boost::asio::async_initiate<CompletionToken, void(std::exception_ptr)>(
        [task = std::move(task)](auto handler) mutable {
            auto executor = boost::asio::get_associated_executor(handler);
            auto work     = boost::asio::make_work_guard(executor);

            async::awaitable_then(
                std::move(task),
                [handler = std::move(handler), executor, work = std::move(work)](
                    async::awaitable_result<void> result) mutable {
                    std::exception_ptr ep;
                    try
                    {
                        result();
                    }
                    catch (...)
                    {
                        ep = std::current_exception();
                    }

                    boost::asio::post(executor, [handler = std::move(handler), ep, work = std::move(work)]() mutable {
                        std::move(handler)(ep);
                        work.reset();
                    });
                });
        },
        std::forward<CompletionToken>(token));
}

/**
 * @brief Value-returning overload. T must be default-constructible for the error path.
 *
 * Completion signature: void(std::exception_ptr, T)
 */
template <typename T, typename CompletionToken>
requires(!std::is_void_v<T>)
auto async_await_task(async::task<T> task, CompletionToken&& token)
{
    static_assert(std::is_default_constructible_v<T>, "async_await_task value type must be default-constructible");

    return boost::asio::async_initiate<CompletionToken, void(std::exception_ptr, T)>(
        [task = std::move(task)](auto handler) mutable {
            auto executor = boost::asio::get_associated_executor(handler);
            auto work     = boost::asio::make_work_guard(executor);

            async::awaitable_then(
                std::move(task),
                [handler = std::move(handler), executor, work = std::move(work)](
                    async::awaitable_result<T> result) mutable {
                    std::exception_ptr ep;
                    auto               value = std::optional<T>{};
                    try
                    {
                        value = result();
                    }
                    catch (...)
                    {
                        ep = std::current_exception();
                    }

                    boost::asio::post(
                        executor,
                        [handler = std::move(handler), ep, value = std::move(value), work = std::move(work)]() mutable {
                            if (ep)
                                std::move(handler)(ep, T{});
                            else
                                std::move(handler)(nullptr, std::move(*value));
                            work.reset();
                        });
                });
        },
        std::forward<CompletionToken>(token));
}

} // namespace fb

#endif // !__FB_ASIO_TASK_H__
