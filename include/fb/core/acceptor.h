#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <map>
#include <ctime>
#include <chrono>
#include <fb/core/abstract.h>
#include <zlib.h>
#include <functional>
#include <iomanip>
#include <fb/core/stream.h>
#include <fb/core/socket.h>
#include <fb/protocol/protocol.h>
#include <fb/core/logger.h>
#include <fb/core/redis.h>
#include <fb/core/mutex.h>
#include <async/awaitable_get.h>

using namespace std::chrono_literals;

namespace fb { namespace base {

template <template<class> class S, class T>
class acceptor : public icontext
{
private:
    fb::threads                                 _threads;
    bool                                        _running = false;
    std::mutex                                  _mutex_exit;
    std::optional<async::task<void>>            _internal_conn_task;

protected:
    std::unique_ptr<boost::asio::thread_pool>   _boost_threads;
    boost::asio::io_context&                    _context;
    std::unique_ptr<fb::internal::socket<>>     _internal;
    fb::redis                                   _redis;
    fb::mutex                                   _mutex;


public:
    fb::base::socket_container<S, T>            sockets;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t num_threads = 0);
    virtual ~acceptor();

private:
    virtual async::task<void>                   handle_work(S<T>* socket);

protected:
    void                                        accept();
    fb::threads&                                threads();
    const fb::threads&                          threads() const;

protected:
    virtual async::task<void>                   handle_start() { co_return;  }
    virtual async::task<bool>                   handle_parse(S<T>& session) = 0;
    virtual T*                                  handle_accepted(S<T>& socket) = 0;
    virtual bool                                handle_connected(S<T>& session) { return true; }
    virtual bool                                handle_disconnected(S<T>& session) { return true; }
    virtual uint8_t                             handle_thread_index(S<T>& socket) const;
    virtual void                                handle_exit() { }

public:
    async::task<void>                           handle_receive(fb::base::socket<T>& socket);
    async::task<void>                           handle_closed(fb::base::socket<T>& socket);

protected:
    void                                        transfer(S<T>& socket, uint32_t ip, uint16_t port, fb::protocol::internal::services from);
    void                                        transfer(S<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::services from);
    void                                        transfer(S<T>& socket, uint32_t ip, uint16_t port, fb::protocol::internal::services from, const fb::ostream& parameter);
    void                                        transfer(S<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::services from, const fb::ostream& parameter);

public:
    void                                        send_stream(S<T>& socket, const fb::ostream& stream, bool encrypt = true, bool wrap = true);
    void                                        send(S<T>& socket, const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true);

public:
    template <typename R>
    async::task<R>                              request(const fb::protocol::internal::header& header, bool encrypt = true, bool wrap = true);
    fb::thread*                                 current_thread();
    async::task<void>                           dispatch(S<T>*, const std::function<async::task<void>(void)>& fn, uint32_t priority = 0);
    async::task<void>                           dispatch(S<T>*, uint32_t priority = 0);
    void                                        run(int thread_size);
    bool                                        running() const;
    async::task<void>                           sleep(const std::chrono::steady_clock::duration& duration);
    void                                        exit();

public:
    operator boost::asio::io_context& () const;
};

} }

namespace fb {

template <typename T>
class acceptor : public fb::base::acceptor<fb::socket, T>
{
private:
    using external_func             = std::function<async::task<bool>(fb::socket<T>&, const std::function<void()>&)>;
    using internal_func             = std::function<async::task<bool>(fb::internal::socket<>&, const std::function<void()>&)>;

private:
    std::map<uint8_t, external_func>    _external_handler;
    std::map<uint8_t, internal_func>    _internal_handler;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t num_threads = 0);
    ~acceptor();

private:
    async::task<bool>               default_handler();
    async::task<void>               connect_internal();
    async::task<void>               co_connect_internal(const std::string& ip, uint16_t port);
    async::task<void>               handle_internal_receive(fb::base::socket<>& socket);

protected:
    virtual bool                    decrypt_policy(uint8_t cmd) const;
    virtual async::task<void>       handle_start();
    async::task<bool>               handle_parse(fb::socket<T>& socket);

public:
    template <typename R>
    void                            bind(int cmd, const std::function<async::task<bool>(fb::socket<T>&, const R&)>& fn);
    template <typename R>
    void                            bind(const std::function<async::task<bool>(fb::socket<T>&, const R&)>& fn);
    template <typename R>
    void                            bind(const std::function<async::task<bool>(fb::internal::socket<>&, const R&)>& fn);
    template <typename R>
    void                            bind();

private:
    async::task<void>               on_internal_disconnected(fb::base::socket<>& socket);

protected:
    virtual async::task<void>       handle_internal_disconnected(fb::base::socket<>& socket);
    virtual async::task<void>       handle_internal_connected();
    virtual async::task<void>       handle_internal_denied(const std::string& error);
};

}

#include "acceptor.hpp"

#endif // !__FB_ACCEPTOR_H__