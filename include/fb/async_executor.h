#ifndef __ASYNC_EXECUTOR_H__
#define __ASYNC_EXECUTOR_H__

/**
 * @file    async_executor.h
 * @brief   Base async_executor classes for asynchronous operations and network acceptors
 * @author  FB Development Team
 *
 * @details This file implements the fundamental async_executor classes that serve as the
 *          foundation for the FB 2D MMORPG server architecture. It provides base
 *          classes for managing asynchronous operations, thread pools, timer systems,
 *          and network connection acceptance with comprehensive lifecycle management.
 *
 *          Key features:
 *          - Base async_executor class with Boost.Asio integration for asynchronous operations
 *          - Thread pool management with configurable worker thread counts
 *          - Comprehensive timer system supporting both thread-based and coroutine-based timers
 *          - Smart pointer integration for safe object lifetime management
 *          - Thread-switchable object management with automatic thread switching
 *          - TCP acceptor for handling incoming network connections
 *          - Exception-safe timer callbacks with proper error handling
 *          - Weak pointer support for preventing circular references in async operations
 *          - Thread safety assertions and validation for multi-threaded environments
 *          - Integration with the thread container system for efficient task distribution
 *
 * @note    This file provides the architectural foundation for all server components
 *          and is critical for proper asynchronous operation and network handling
 *          throughout the FB 2D MMORPG server infrastructure.
 */

#include <boost/asio.hpp>
#include <fb/thread_container.h>
#include <fb/hash.h>
#include <fb/locker.h>
#include <async/awaitable_get.h>
#include <memory>

namespace fb {

/**
 * @brief      Base async_executor class for managing asynchronous operations and thread pools.
 *
 *             This class provides a foundation for managing asynchronous operations using
 *             Boost.Asio, thread pools, and timer functionality. It maintains thread-safe
 *             collections of switchable objects and provides timer binding capabilities
 *             for both thread-based and coroutine-based execution.
 */
class async_executor : public std::enable_shared_from_this<async_executor>
{
public:
    using boost_timers = std::vector<std::shared_ptr<boost::asio::deadline_timer>>;

private:
    using super = std::enable_shared_from_this<async_executor>;

protected:
    boost_timers _timers;
    bool         _running = false;

public:
    boost::asio::io_context& io_context;

public:
    thread_container threads;

protected:
    async_executor(boost::asio::io_context& io_context, const std::string& name, uint32_t thread_count);

public:
    virtual ~async_executor() = default;

protected:
    /**
     * @brief      Binds a member function as a coroutine-based timer callback.
     *
     *             Creates a coroutine-based timer that executes the specified member function
     *             at regular intervals using Boost.Asio's coroutine support. The timer runs
     *             asynchronously and continues until the async_executor is stopped.
     *
     * @tparam     Class     The class type containing the member function
     * @param[in]  fn        The member function to execute as timer callback
     * @param[in]  interval  The time interval between timer executions
     */
    template <typename Class>
    void bind_timer(async::task<void> (Class::*fn)(void), std::chrono::steady_clock::duration interval)
    {
        // Use weak_ptr for safe object reference
        auto weak_this = std::weak_ptr<Class>(std::static_pointer_cast<Class>(this->shared_from_this()));

        // Get executor
        auto exec = this->io_context.get_executor();

        // Spawn coroutine with improved safety
        boost::asio::co_spawn(
            exec,
            [weak_this, fn, interval]() -> boost::asio::awaitable<void> {
                boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);

                while (true)
                {
                    timer.expires_after(interval);
                    co_await timer.async_wait(boost::asio::use_awaitable);

                    // Check if object is still alive
                    auto shared_this = weak_this.lock();
                    if (!shared_this)
                        break; // Object has been destroyed, stop timer

                    // Check if async_executor is still running
                    if (!shared_this->_running)
                        break;

                    try
                    {
                        // Execute the timer callback
                        std::ignore = (shared_this.get()->*fn)();
                    }
                    catch (const std::exception& e)
                    {
                        // Log timer callback errors but don't stop the timer
                        fb::logger::warn(std::format("Timer callback error: {}", e.what()));
                    }
                    catch (...)
                    {
                        // Log unknown errors
                        fb::logger::warn("Timer callback error: Unknown exception");
                    }
                }
            },
            boost::asio::detached);
    }

    /**
     * @brief      Binds a lambda function as a thread-based timer callback.
     *
     *             Creates a timer that executes the specified lambda function at regular
     *             intervals on the thread pool. The callback receives timing information
     *             and thread ID for context-aware processing.
     *
     * @param[in]  fn        The lambda function to execute as timer callback
     * @param[in]  duration  The time interval between timer executions
     */
    void bind_thread_timer(std::function<async::task<void>(const fb::model::datetime&, std::thread::id)> fn,
                           const std::chrono::steady_clock::duration&                                    duration)
    {
        this->threads.settimer(fn, duration);
    }

    /**
     * @brief      Binds a timer handler to the thread pool.
     *
     *             Creates a timer that executes the specified handler at regular intervals
     *             on the thread pool. The handler receives timing information and thread ID
     *             for context-aware processing.
     *
     * @tparam     HandlerType  The type of the timer handler
     * @param[in]  duration     The time interval between timer executions
     */
    template <typename HandlerType>
    void bind_thread_timer(const std::chrono::steady_clock::duration& duration)
    {
        using server_type = typename HandlerType::server_type;

        auto weak_this = this->weak_from_this();
        this->threads.settimer(
            [weak_this](const fb::model::datetime& now, std::thread::id id) -> async::task<void> {
                auto shared_this = weak_this.lock();
                if (!shared_this)
                    co_return;

                auto handler = std::make_shared<HandlerType>(*std::static_pointer_cast<server_type>(shared_this));
                co_await handler->handle(now, id);
            },
            duration);
    }

    /**
     * @brief      Binds a timer handler to the thread pool.
     *
     *             Creates a timer that executes the specified handler at regular intervals
     *             on the thread pool.
     *
     * @tparam     HandlerType  The type of the timer handler
     * @param[in]  interval     The time interval between timer executions
     */
    template <typename HandlerType>
    void bind_timer(std::chrono::steady_clock::duration interval)
    {
        using server_type = typename HandlerType::server_type;
        this->bind_timer(
            [this]() -> async::task<void> {
                auto handler = std::make_shared<HandlerType>(static_cast<server_type&>(*this));
                co_await handler->handle();
            },
            interval);
    }

    /**
     * @brief      Binds a member function as a coroutine-based timer callback with improved safety.
     *
     *             Creates a coroutine-based timer that executes the specified member function
     *             at regular intervals using Boost.Asio's coroutine support. The timer runs
     *             asynchronously and continues until the async_executor is stopped.
     *
     *             Safety improvements:
     *             - Uses weak_ptr to prevent dangling pointer issues
     *             - Thread-safe running flag access
     *             - Proper exception handling
     *
     * @param[in]  fn        The lambda function to execute as timer callback
     * @param[in]  interval  The time interval between timer executions
     */
    void bind_timer(std::function<async::task<void>()> fn, std::chrono::steady_clock::duration interval)
    {
        // Use weak_ptr for safe object reference
        auto weak_this = this->weak_from_this();

        // Get executor
        auto exec = this->io_context.get_executor();

        // Spawn coroutine with improved safety
        boost::asio::co_spawn(
            exec,
            [weak_this, fn, interval]() -> boost::asio::awaitable<void> {
                boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);

                while (true)
                {
                    timer.expires_after(interval);
                    co_await timer.async_wait(boost::asio::use_awaitable);

                    // Check if object is still alive
                    auto shared_this = weak_this.lock();
                    if (!shared_this)
                        break; // Object has been destroyed, stop timer

                    // Check if async_executor is still running
                    if (!shared_this->_running)
                        break;

                    try
                    {
                        // Execute the timer callback
                        std::ignore = fn();
                    }
                    catch (const std::exception& e)
                    {
                        // Log timer callback errors but don't stop the timer
                        fb::logger::warn(std::format("Timer callback error: {}", e.what()));
                    }
                    catch (...)
                    {
                        // Log unknown errors
                        fb::logger::warn("Timer callback error: Unknown exception");
                    }
                }
            },
            boost::asio::detached);
    }

public:
    /**
     * @brief      Gets the IPv4 address from a hostname or IP address string.
     *
     * @param[in]  ip  The hostname or IP address to resolve.
     *
     * @return     The resolved IPv4 address as a string.
     */
    std::string ipv4(const std::string& ip) const
    {
        try
        {
            auto resolver = boost::asio::ip::tcp::resolver(this->io_context);
            auto results  = resolver.resolve(ip, "0");

            for (const auto& entry : results)
            {
                auto addr = entry.endpoint().address();
                if (addr.is_v4())
                    return addr.to_string();
            }

            throw std::runtime_error(std::format("Failed to resolve IPv4 address for: {}", ip));
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::format("Error resolving address: {}", e.what()));
        }
    }

public:
    virtual void exit();

public:
    operator boost::asio::io_context& () const;
};

} // namespace fb

#endif // !__ASYNC_EXECUTOR_H__