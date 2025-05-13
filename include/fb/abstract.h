#ifndef __ABSTRACT_H__
#define __ABSTRACT_H__

#include <boost/asio.hpp>
#include <fb/thread_container.h>
#include <fb/hash.h>
#include <fb/locker.h>

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
    void bind_timer(async::task<void> (Class::*fn)(void), const std::chrono::steady_clock::duration& duration)
    {
        auto cfunc = std::bind(fn, static_cast<Class*>(this));
        auto timer = std::make_shared<boost::asio::deadline_timer>(this->_boost_context, boost::posix_time::seconds(1));
        this->_timers.push_back(timer);

        auto callback_ptr = std::make_shared<std::function<void(const boost::system::error_code&)>>();
        auto callback     = [=, this](const boost::system::error_code& ec) {
            if (ec || !this->_running)
                return;

            async::awaitable_then(cfunc(), [timer, callback_ptr, duration](async::awaitable_result<void> result) {
                timer->expires_at(timer->expires_at() +
                                  boost::posix_time::milliseconds(
                                      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()));
                timer->async_wait(*callback_ptr.get());
            });
        };
        *callback_ptr = callback;
        timer->async_wait(*callback_ptr.get());
    }

public:
    virtual ~context() = default;

public:
    virtual void exit();

public:
    void              push_alive(const fb::thread_switchable& obj);
    void              pop_alive(const fb::thread_switchable& obj);
    bool              alive(const fb::thread_switchable& obj) const;
    async::task<void> update_thread(const fb::thread_switchable& obj);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  obj   The object
     * @param[in]  fn    The function
     * @param[in]  n     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    int builtin(thread_switchable& obj, fb::lua::context* lua, int n, const std::function<void()>& fn);

    /**
     * @brief      { function_description }
     *
     * @param      obj   The object
     * @param      lua   The lua
     * @param[in]  n     { parameter_description }
     * @param[in]  fn    The function
     *
     * @return     { description_of_the_return_value }
     */
    int
    builtin_async(thread_switchable& obj, fb::lua::context* lua, int n, const std::function<async::task<void>()>& fn);

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