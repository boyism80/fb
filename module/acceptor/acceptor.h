#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <thread>
#include <map>
#include <ctime>
#include <boost/asio.hpp>
#include <zlib.h>
#include "module/stream/stream.h"
#include "module/socket/socket.h"
#include "protocol/protocol.h"
#include "module/console/console.h"

namespace fb { namespace base {

template <template<class> class S, class T>
class acceptor : public boost::asio::ip::tcp::acceptor
{
protected:
    boost::asio::io_context&    _context;
    uint8_t                     _accept_delay;
    fb::internal::socket<>*     _internal;

public:
    fb::base::socket_container<S, T>  sockets;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay);
    ~acceptor();

protected:
    void                        accept();
    void                        handle_accept_timer(const boost::system::error_code& e, S<T>* socket, boost::asio::steady_timer* timer);

protected:
    virtual void                handle_parse(S<T>& session) = 0;
    virtual T*                  handle_accepted(S<T>& socket) = 0;
    virtual bool                handle_connected(S<T>& session) { return true; }
    virtual bool                handle_disconnected(S<T>& session) { return true; }

public:
    void                        handle_receive(fb::base::socket<T>& socket);
    void                        handle_closed(fb::base::socket<T>& socket);

protected:
    void                        transfer(S<T>& socket, uint32_t ip, uint16_t port);
    void                        transfer(S<T>& socket, const std::string& ip, uint16_t port);
    void                        transfer(S<T>& socket, uint32_t ip, uint16_t port, const fb::ostream& parameter);
    void                        transfer(S<T>& socket, const std::string& ip, uint16_t port, const fb::ostream& parameter);

public:
    void                        send_stream(S<T>& socket, const fb::ostream& stream, bool encrypt = true, bool wrap = true);
    void                        send(S<T>& socket, const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true);

public:
    operator boost::asio::io_context& () const;
};

} }

typedef struct __INTERNAL_CONNECTION_TAG
{
    std::string                                     ip;
    uint16_t                                        port;
    std::function<void(fb::base::socket<>&, bool)>  handle_connected;
    std::function<void()>                           handle_disconnected;
} INTERNAL_CONNECTION;

namespace fb { namespace internal {

template <typename T, typename D>
void parse(fb::internal::socket<T>& socket, D& dict)
{
    static constexpr uint8_t base_size = sizeof(uint16_t);
    auto& in_stream = socket.in_stream();

    while(true)
    {
        try
        {
            if(in_stream.readable_size() < base_size)
                throw std::exception();

            auto size = in_stream.read_u16();
            if(size > in_stream.capacity())
                throw std::exception();

            if(in_stream.readable_size() < size)
                throw std::exception();

            auto cmd = in_stream.read_8();
            auto found = dict.find(cmd);
            if(found == dict.end())
                throw std::exception();

            found->second(socket);

            in_stream.reset();
            in_stream.shift(base_size + size);
            in_stream.flush();
        }
        catch(...)
        {
            break;
        }
    }

    in_stream.reset();
}

} }

namespace fb {

template <typename T>
class acceptor : public fb::base::acceptor<fb::socket, T>
{
private:
    typedef std::function<bool(fb::socket<T>&)>             public_handler;
    typedef std::function<bool(fb::internal::socket<>&)>    private_handler;

private:
    std::map<uint8_t, public_handler>   _public_handler_dict;
    std::map<uint8_t, private_handler>  _private_handler_dict;
    const INTERNAL_CONNECTION           _internal_connection;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay, const INTERNAL_CONNECTION& internal_connection);
    ~acceptor();

private:
    void                        connect_internal();
    bool                        call(fb::socket<T>& socket, uint8_t cmd);

protected:
    void                        handle_parse(fb::socket<T>& socket);

public:
    template <typename R>
    void                        bind(uint8_t cmd, std::function<bool(fb::socket<T>&, R&)> fn);

    template <typename R>
    void                        bind(std::function<bool(fb::internal::socket<>&, R&)> fb);
};

}

#include "acceptor.hpp"

#endif // !__FB_ACCEPTOR_H__