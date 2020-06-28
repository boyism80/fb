#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <thread>
#include <map>
#include <ctime>
#include <boost/asio.hpp>
#include "zlib/zlib.h"
#include "module/stream/stream.h"
#include "module/socket/socket.h"
#include "module/crypto/cryptor.h"

namespace fb {

class base_acceptor : public boost::asio::ip::tcp::acceptor
{
protected:
    base_acceptor(boost::asio::io_context& context, uint16_t port);
    ~base_acceptor();

public:
    virtual void            handle_receive(fb::socket& socket) {}
    virtual void            handle_send(fb::socket& socket) {}
    virtual void            handle_disconnected(fb::socket& socket) {}
};


template <class T>
class acceptor : public base_acceptor
{
private:
    session_map<T>          _session_map;
    std::map<uint8_t, std::function<bool(T&)>> _response_table;

public:
    session_container<T>    sessions;

public:
    acceptor(boost::asio::io_context& context, uint16_t port);
    ~acceptor();

private:
    bool                    call_handle(T& session, uint8_t cmd);
    void                    accept();
    void                    handle_parse(T& session);
    static fb::ostream      make_transfer_stream(const fb::cryptor& crt, uint32_t ip, uint16_t port, const fb::ostream* parameter = nullptr);

protected:
    virtual T*              handle_alloc_session(fb::socket* socket) = 0;
    virtual bool            handle_connected(T& session) { return true; }
    virtual bool            handle_disconnected(T& session) { return true; }

public:
    void                    handle_receive(fb::socket& socket);
    void                    handle_disconnected(fb::socket& socket);

protected:
    void                    register_fn(uint8_t cmd, std::function<bool(T&)> fn);
    void                    transfer(T& base_session, uint32_t ip, uint16_t port);
    void                    transfer(T& base_session, uint32_t ip, uint16_t port, const fb::ostream& parameter);

public:
    void                    send_stream(fb::base& base, const fb::ostream& stream, bool encrypt = true, bool wrap = true);
};

#include "acceptor.hpp"

}

#endif // !__FB_ACCEPTOR_H__