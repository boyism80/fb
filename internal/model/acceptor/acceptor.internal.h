#ifndef __ACCEPTOR_INTERNAL_H__
#define __ACCEPTOR_INTERNAL_H__

#include "model/session/session.internal.h"
#include <zlib.h>
#include "protocol/internal.h"
#include "module/console/console.h"
#include "module/acceptor/acceptor.h"
#include "model/table/table.internal.h"
#include "model/exception/exception.h"
#include "module/config/config.h"

namespace fb { namespace internal {

class user
{
public:
    const uint16_t                  group = 0;

public:
    user(uint16_t group) : 
        group(group)
    { }
};

class acceptor : public fb::base::acceptor<fb::internal::socket, fb::internal::session>
{
public:
    using service                   = fb::internal::socket<fb::internal::session>;
    using handler                   = std::function<bool(service&)>;
    using subscriber_container      = std::map<uint8_t, service*>;

private:
    std::map<uint8_t, handler>      _handler_dict;
    service*                        _gateway = nullptr;
    service*                        _login   = nullptr;
    subscriber_container            _games;
    std::map<std::string, user*>    _users;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, std::chrono::seconds delay);
    ~acceptor();

private:
    service*                        get(fb::protocol::internal::services type, uint8_t group = 0xFF);

protected:
    bool                            handle_parse(fb::internal::socket<fb::internal::session>& socket, const std::function<bool(fb::internal::socket<fb::internal::session>&)>& callback);
    
public:
    fb::internal::session*          handle_accepted(fb::internal::socket<fb::internal::session>& socket);

public:
    bool                            handle_connected(fb::internal::socket<fb::internal::session>& session);
    bool                            handle_disconnected(fb::internal::socket<fb::internal::session>& session);

public:
    template <typename R>
    void                            bind(const std::function<bool(fb::internal::socket<fb::internal::session>&, R&)>& fn)
    {
        _handler_dict[R::id] = [this, fn] (fb::internal::socket<fb::internal::session>& socket)
        {
            auto&   in_stream = socket.in_stream();
            R       header;
            header.deserialize(in_stream);
            return fn(socket, header);
        };
    }

public:
    bool                            handle_subscribe(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::subscribe&);
    bool                            handle_transfer(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::transfer&);
    bool                            handle_login(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::login&);
    bool                            handle_logout(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::logout&);
    bool                            handle_whisper(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::whisper&);
    bool                            handle_shutdown(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::shutdown&);
};

} }

#endif // !__ACCEPTOR_INTERNAL_H__
