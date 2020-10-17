#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <thread>
#include <map>
#include <ctime>
#include <boost/asio.hpp>
#include <zlib.h>
#include "module/stream/stream.h"
#include "module/socket/socket.h"
#include "module/console/console.h"
#include "protocol/protocol.h"

namespace fb { namespace base {

template <template<class> class S, class T>
class acceptor : public boost::asio::ip::tcp::acceptor
{
protected:
    boost::asio::io_context&    _context;
    uint8_t                     _accept_delay;

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
    void                        send(S<T>& socket, const fb::protocol::base::response& response, bool encrypt = true, bool wrap = true);
};

} }


namespace fb {

template <typename T>
class acceptor : public fb::base::acceptor<fb::socket, T>
{
private:
    typedef std::function<bool(fb::socket<T>&)> handler;

private:
    std::map<uint8_t, handler>  _handler_dict;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay);
    ~acceptor();

private:
    bool                        call(fb::socket<T>& socket, uint8_t cmd);

protected:
    void                        handle_parse(fb::socket<T>& socket);

public:
    template <typename R>
    void                        bind(uint8_t cmd, std::function<bool(fb::socket<T>&, R&)> fn);
};

}

#include "acceptor.hpp"

#endif // !__FB_ACCEPTOR_H__