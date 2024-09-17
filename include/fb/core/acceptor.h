#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <map>
#include <ctime>
#include <chrono>
#include <fb/core/abstract.h>
#include <zlib.h>
#include <functional>
#include <iomanip>
#include <httplib.h>
#include <fb/core/stream.h>
#include <fb/core/socket.h>
#include <fb/protocol/protocol.h>
#include <fb/core/logger.h>
#include <fb/core/redis.h>
#include <fb/core/mutex.h>
#include <async/awaitable_get.h>

using namespace std::chrono_literals;

namespace fb {

template <typename T>
class acceptor : public icontext
{
private:
    using handle_func = std::function<async::task<bool>(fb::socket<T>&, const std::function<void()>&)>;

private:
    std::map<uint8_t, handle_func>              _handler;
    fb::threads                                 _threads;
    bool                                        _running = false;
    std::mutex                                  _mutex_exit;

protected:
    std::unique_ptr<boost::asio::thread_pool>   _boost_threads;
    boost::asio::io_context&                    _context;
    fb::redis                                   _redis;
    fb::mutex                                   _mutex;


public:
    fb::socket_container<T>                     sockets;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t num_threads = 0);
    virtual ~acceptor();

private:
    virtual async::task<void>                   handle_work(fb::socket<T>* socket);

protected:
    void                                        accept();
    fb::threads&                                threads();
    const fb::threads&                          threads() const;

protected:
    virtual bool                                decrypt_policy(uint8_t cmd) const;
    virtual async::task<void>                   handle_start() { co_return;  }
    virtual T*                                  handle_accepted(fb::socket<T>& socket) = 0;
    virtual bool                                handle_connected(fb::socket<T>& session) { return true; }
    virtual bool                                handle_disconnected(fb::socket<T>& session) { return true; }
    virtual uint8_t                             handle_thread_index(fb::socket<T>& socket) const;
    virtual void                                handle_exit() { }
    async::task<bool>                           handle_parse(fb::socket<T>& socket);

public:
    async::task<void>                           handle_receive(fb::socket<T>& socket);
    async::task<void>                           handle_closed(fb::socket<T>& socket);

protected:
    void                                        transfer(fb::socket<T>& socket, uint32_t ip, uint16_t port, fb::protocol::internal::services from);
    void                                        transfer(fb::socket<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::services from);
    void                                        transfer(fb::socket<T>& socket, uint32_t ip, uint16_t port, fb::protocol::internal::services from, const fb::ostream& parameter);
    void                                        transfer(fb::socket<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::services from, const fb::ostream& parameter);

public:
    void                                        send_stream(fb::socket<T>& socket, const fb::ostream& stream, bool encrypt = true, bool wrap = true);
    void                                        send(fb::socket<T>& socket, const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true);

private:
    async::task<httplib::Result>                post_async(const std::string& host, const std::string& path, httplib::Headers headers, const void* bytes, size_t size);

public:
    template <typename Request, typename Response>
    async::task<Response>                       post_async(const std::string& host, const std::string& path, /* httplib::Headers headers,  */const Request& body);

public:
    fb::thread*                                 current_thread();
    async::task<void>                           dispatch(fb::socket<T>*, const std::function<async::task<void>(void)>& fn, uint32_t priority = 0);
    async::task<void>                           dispatch(fb::socket<T>*, uint32_t priority = 0);
    void                                        run(int thread_size);
    bool                                        running() const;
    async::task<void>                           sleep(const std::chrono::steady_clock::duration& duration);
    void                                        exit();

public:
    template <typename R>
    void                                        bind(int cmd, const std::function<async::task<bool>(fb::socket<T>&, const R&)>& fn);
    template <typename R>
    void                                        bind(const std::function<async::task<bool>(fb::socket<T>&, const R&)>& fn);

public:
    operator boost::asio::io_context& () const;
};

}

#include "acceptor.hpp"

#endif // !__FB_ACCEPTOR_H__