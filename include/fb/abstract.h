#ifndef __ABSTRACT_H__
#define __ABSTRACT_H__

#include <boost/asio.hpp>
#include <fb/thread_container.h>
#include <fb/hash.h>
#include <fb/locker.h>
#include <async/awaitable_get.h>

namespace fb {

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
     * @brief      { function_description }
     *
     * @param      func      The function
     * @param[in]  duration  The duration
     *
     * @tparam     Func      { description }
     */
    template <typename Class>
    void bind_thread_timer(async::task<void> (Class::*fn)(const fb::model::datetime&, std::thread::id),
                           const std::chrono::steady_clock::duration& duration)
    {
        auto c_func = std::bind(fn, static_cast<Class*>(this), std::placeholders::_1, std::placeholders::_2);
        this->threads.settimer(c_func, duration);
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  fn        The function
     * @param[in]  duration  The duration
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

class acceptable : public context, public boost::asio::ip::tcp::acceptor
{
protected:
    acceptable(boost::asio::io_context& context, const std::string& name, uint32_t thread_count, uint16_t port);

public:
    virtual ~acceptable() = default;
};

} // namespace fb

#endif