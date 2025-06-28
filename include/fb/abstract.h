#ifndef __ABSTRACT_H__
#define __ABSTRACT_H__

/**
 * @file    abstract.h
 * @brief   Base context classes for asynchronous operations and network acceptors
 * @author  FB Development Team
 *
 * @details This file implements the fundamental context classes that serve as the
 *          foundation for the FB 2D MMORPG server architecture. It provides base
 *          classes for managing asynchronous operations, thread pools, timer systems,
 *          and network connection acceptance with comprehensive lifecycle management.
 *
 *          Key features:
 *          - Base context class with Boost.Asio integration for asynchronous operations
 *          - Thread pool management with configurable worker thread counts
 *          - Comprehensive timer system supporting both thread-based and coroutine-based timers
 *          - Smart pointer integration for safe object lifetime management
 *          - Thread-switchable object management with automatic context switching
 *          - TCP acceptor context for handling incoming network connections
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
 * @brief      Base context class for managing asynchronous operations and thread pools.
 *
 *             This class provides a foundation for managing asynchronous operations using
 *             Boost.Asio, thread pools, and timer functionality. It maintains thread-safe
 *             collections of switchable objects and provides timer binding capabilities
 *             for both thread-based and coroutine-based execution.
 */
class context : public std::enable_shared_from_this<context>
{
public:
    using boost_timers = std::vector<std::shared_ptr<boost::asio::deadline_timer>>;

private:
    using super = std::enable_shared_from_this<context>;

protected:
    boost_timers _timers;
    bool         _running = false;

public:
    boost::asio::io_context& io_context;

public:
    thread_container threads;

protected:
    context(boost::asio::io_context& context, const std::string& name, uint32_t thread_count);

protected:
    /**
     * @brief      Binds a member function as a thread-based timer callback.
     *
     *             Creates a timer that executes the specified member function at regular
     *             intervals on the thread pool. The callback receives timing information
     *             and thread ID for context-aware processing.
     *
     * @tparam     Class     The class type containing the member function
     * @param[in]  fn        The member function to execute as timer callback
     * @param[in]  duration  The time interval between timer executions
     */
    template <typename Class>
    void bind_thread_timer(async::task<void> (Class::*fn)(const fb::model::datetime&, std::thread::id),
                           const std::chrono::steady_clock::duration& duration)
    {
        auto c_func = std::bind(fn, static_cast<Class*>(this), std::placeholders::_1, std::placeholders::_2);
        this->threads.settimer(c_func, duration);
    }

    /**
     * @brief      Binds a member function as a coroutine-based timer callback with improved safety.
     *
     *             Creates a coroutine-based timer that executes the specified member function
     *             at regular intervals using Boost.Asio's coroutine support. The timer runs
     *             asynchronously and continues until the context is stopped.
     *
     *             Safety improvements:
     *             - Uses weak_ptr to prevent dangling pointer issues
     *             - Thread-safe running flag access
     *             - Proper exception handling
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

                    // Check if context is still running
                    if (!shared_this->_running)
                        break;

                    try
                    {
                        // Execute the timer callback
                        async::awaitable_get((shared_this.get()->*fn)());
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
    virtual ~context() = default;

public:
    virtual void exit();

public:
    operator boost::asio::io_context& () const;
};

/**
 * @brief      TCP acceptor context for handling incoming network connections.
 *
 *             This class extends the base context to provide TCP server functionality,
 *             combining asynchronous I/O operations with thread pool management.
 *             It inherits from both context and Boost.Asio's TCP acceptor to provide
 *             a complete server foundation.
 */
class acceptable : public context, public boost::asio::ip::tcp::acceptor
{
private:
    using super_context  = context;
    using super_acceptor = boost::asio::ip::tcp::acceptor;

protected:
    /**
     * @brief      Constructs a new TCP acceptor context.
     *
     *             Creates a new acceptor context that listens on the specified port
     *             and manages a thread pool for handling connections.
     *
     * @param[in]  context       The Boost.Asio I/O context for asynchronous operations
     * @param[in]  name          The name identifier for this acceptor context
     * @param[in]  thread_count  The number of worker threads to create
     * @param[in]  port          The TCP port number to listen on
     */
    acceptable(boost::asio::io_context& context, const std::string& name, uint32_t thread_count, uint16_t port);

public:
    /**
     * @brief      Destroys the acceptor context and cleans up resources.
     *
     *             Properly shuts down the acceptor, closes any open connections,
     *             and releases all associated resources.
     */
    virtual ~acceptable() = default;
};

} // namespace fb

#endif // !__ABSTRACT_H__