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

namespace fb {

template <class T>
class acceptor : public boost::asio::ip::tcp::acceptor
{
private:
    uint8_t                 _accept_delay;
    std::map<uint8_t, std::function<bool(fb::socket<T>&)>> _handler_dict;

public:
    fb::socket_container<T> sockets;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay);
    ~acceptor();

private:
    bool                    call_handle(fb::socket<T>& session, uint8_t cmd);
    void                    accept();
    void                    handle_parse(fb::socket<T>& session);

protected:
    virtual T*              handle_accepted(fb::socket<T>& socket) = 0;
    virtual bool            handle_connected(fb::socket<T>& session) { return true; }
    virtual bool            handle_disconnected(fb::socket<T>& session) { return true; }

public:
    void                    handle_receive(fb::socket<T>& socket);
    void                    handle_closed(fb::socket<T>& socket);
    virtual void            handle_send(fb::socket<T>& socket) {}

protected:
    template <typename R>
    void                    bind(uint8_t cmd, std::function<bool(fb::socket<T>&, R&)> fn);
    void                    transfer(fb::socket<T>& socket, uint32_t ip, uint16_t port);
    void                    transfer(fb::socket<T>& socket, const std::string& ip, uint16_t port);
    void                    transfer(fb::socket<T>& socket, uint32_t ip, uint16_t port, const fb::ostream& parameter);
    void                    transfer(fb::socket<T>& socket, const std::string& ip, uint16_t port, const fb::ostream& parameter);

public:
    void                    send_stream(fb::socket<T>& socket, const fb::ostream& stream, bool encrypt = true, bool wrap = true, bool async = true);
    void                    send(fb::socket<T>& socket, const fb::protocol::base::response& response, bool encrypt = true, bool wrap = true, bool async = true);
};

#include "acceptor.hpp"

}

#endif // !__FB_ACCEPTOR_H__