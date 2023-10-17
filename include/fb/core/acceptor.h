#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <map>
#include <ctime>
#include <chrono>
#include <boost/asio.hpp>
#include <zlib.h>
#include <functional>
#include <iomanip>
#include <fb/core/stream.h>
#include <fb/core/socket.h>
#include <fb/protocol/protocol.h>
#include <fb/core/logger.h>
#include <fb/core/thread.h>

using namespace std::chrono_literals;

namespace fb { namespace base {

template <template<class> class S, class T>
class acceptor : public boost::asio::ip::tcp::acceptor
{
private:
    fb::threads                                 _threads;
    bool                                        _running = false;
    std::mutex                                  _mutex_exit;

protected:
    std::unique_ptr<boost::asio::thread_pool>   _boost_threads;
    boost::asio::io_context&                    _context;
    std::unique_ptr<fb::internal::socket<>>     _internal;


public:
    fb::base::socket_container<S, T>            sockets;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t num_threads = 0);
    virtual ~acceptor();

private:
    virtual void                                handle_work(S<T>* socket, uint8_t id);

protected:
    void                                        accept();
    fb::threads&                                threads();
    const fb::threads&                          threads() const;

    template <typename R>
    fb::task                                    co_internal_request(fb::awaitable<R>& awaitable, const fb::protocol::internal::header& header, bool encrypt, bool wrap);

protected:
    virtual void                                handle_start() {}
    virtual bool                                handle_parse(S<T>& session, const std::function<bool(S<T>&)>& fn) = 0;
    virtual T*                                  handle_accepted(S<T>& socket) = 0;
    virtual bool                                handle_connected(S<T>& session) { return true; }
    virtual bool                                handle_disconnected(S<T>& session) { return true; }
    virtual uint8_t                             handle_thread_index(S<T>& socket) const;
    virtual void                                handle_exit() { }

public:
    void                                        handle_receive(fb::base::socket<T>& socket);
    void                                        handle_closed(fb::base::socket<T>& socket);

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
    fb::awaitable<R>                            request(const fb::protocol::internal::header& header, bool encrypt = true, bool wrap = true);
    fb::thread*                                 current_thread();
    bool                                        precedence(S<T>*, fb::queue_callback&& fn);
    fb::awaitable<void>                         precedence(S<T>*);
    bool                                        dispatch(S<T>*, fb::queue_callback&& fn);
    fb::awaitable<void>                         dispatch(S<T>*);
    void                                        run(int thread_size);
    bool                                        running() const;
    fb::awaitable<void>                         sleep(const std::chrono::steady_clock::duration& duration);
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
    using public_handler                = std::function<bool(fb::socket<T>&)>;
    using private_handler               = std::function<bool(fb::internal::socket<>&)>;

private:
    std::map<uint8_t, public_handler>   _public_handler_dict;
    std::map<uint8_t, private_handler>  _private_handler_dict;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t num_threads = 0);
    ~acceptor();

private:
    fb::task                    connect_internal();
    bool                        call(fb::socket<T>& socket, uint8_t cmd);
    fb::awaitable<void>         co_connect_internal(const std::string& ip, uint16_t port);
    bool                        handle_internal_receive(fb::base::socket<>& socket);

protected:
    virtual bool                is_decrypt(uint8_t cmd) const;
    bool                        handle_parse(fb::socket<T>& socket, const std::function<bool(fb::socket<T>&)>& callback);

public:
    template <typename R>
    void                        bind(int cmd, const std::function<bool(fb::socket<T>&, R&)>& fn);
    template <typename R>
    void                        bind(const std::function<bool(fb::socket<T>&, R&)>& fn);
    template <typename R>
    void                        bind(const std::function<bool(fb::internal::socket<>&, R&)>& fn);
    template <typename R>
    void                        bind();

private:
    void                        on_internal_connected();
    void                        on_internal_disconnected(fb::base::socket<>& socket);

protected:
    virtual void                handle_internal_disconnected(fb::base::socket<>& socket);
    virtual void                handle_internal_connected();
    virtual void                handle_internal_denied(std::exception& e);
};

}

#include "acceptor.hpp"

#endif // !__FB_ACCEPTOR_H__