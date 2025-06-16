#ifndef __ABSTRACT_H__
#define __ABSTRACT_H__

#include <boost/asio.hpp>
#include <fb/thread_container.h>
#include <fb/hash.h>
#include <fb/locker.h>
#include <async/awaitable_get.h>

namespace fb {

/**
 * @brief      Base context class for managing asynchronous operations and thread pools.
 *
 *             This class provides a foundation for managing asynchronous operations using
 *             Boost.Asio, thread pools, and timer functionality. It maintains thread-safe
 *             collections of switchable objects and provides timer binding capabilities
 *             for both thread-based and coroutine-based execution.
 */
class context
{
public:
    using hash_switchable = fb::hash<fb::locker<std::unordered_set<fb::thread_switchable*>>>;
    using boost_timers    = std::vector<std::shared_ptr<boost::asio::deadline_timer>>;

private:
    hash_switchable _hash_switchable;
    boost_timers    _timers;

protected:
    boost::asio::io_context& _boost_context;
    bool                     _running = false;

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
     * @brief      Binds a member function as a coroutine-based timer callback.
     *
     *             Creates a coroutine-based timer that executes the specified member function
     *             at regular intervals using Boost.Asio's coroutine support. The timer runs
     *             asynchronously and continues until the context is stopped.
     *
     * @tparam     Class     The class type containing the member function
     * @param[in]  fn        The member function to execute as timer callback
     * @param[in]  interval  The time interval between timer executions
     */
    template <typename Class>
    void bind_timer(async::task<void> (Class::*fn)(void), std::chrono::steady_clock::duration interval)
    {
        // 멤버 함수 바인딩
        auto cfunc = std::bind(fn, static_cast<Class*>(this));

        // executor 얻기
        auto exec = this->_boost_context.get_executor();

        // 코루틴 스폰
        boost::asio::co_spawn(
            exec,
            [this, cfunc, interval]() -> boost::asio::awaitable<void> {
                boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
                while (this->_running)
                {
                    timer.expires_after(interval);
                    co_await timer.async_wait(boost::asio::use_awaitable);
                    async::awaitable_get(cfunc());
                }
            },
            boost::asio::detached);
    }

public:
    virtual ~context() = default;

public:
    virtual void exit();

public:
    void              push_alive(const fb::thread_switchable& obj);
    void              pop_alive(const fb::thread_switchable& obj);
    bool              alive(const fb::thread_switchable& obj) const;
    async::task<void> switch_thread(const fb::thread_switchable& obj);

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