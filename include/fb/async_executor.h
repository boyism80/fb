#ifndef __ASYNC_EXECUTOR_H__
#define __ASYNC_EXECUTOR_H__

#include <boost/asio.hpp>
#include <fb/thread_container.h>
#include <fb/hash.h>
#include <fb/locker.h>
#include <async/awaitable_get.h>
#include <memory>

namespace fb {

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
    async_executor(boost::asio::io_context& io_context, std::string_view name, uint32_t thread_count);

public:
    virtual ~async_executor() = default;

protected:
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

    void bind_thread_timer(std::function<async::task<void>(const fb::model::datetime&, std::thread::id)>&& fn,
                           const std::chrono::steady_clock::duration&                                      duration)
    {
        this->threads.settimer(std::move(fn), duration);
    }

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

    void bind_timer(std::function<async::task<void>()>&& fn, std::chrono::steady_clock::duration interval)
    {
        // Use weak_ptr for safe object reference
        auto weak_this = this->weak_from_this();

        // Get executor
        auto exec = this->io_context.get_executor();

        // Spawn coroutine with improved safety
        boost::asio::co_spawn(
            exec,
            [weak_this, fn = std::move(fn), interval]() -> boost::asio::awaitable<void> {
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
    std::string ipv4(std::string_view ip) const
    {
        try
        {
            auto resolver = boost::asio::ip::tcp::resolver(this->io_context);
            auto results  = resolver.resolve(std::string(ip), "0");

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