#ifndef __ACCEPTOR_INTERNAL_H__
#define __ACCEPTOR_INTERNAL_H__

#include <fb/internal/session.h>
#include <zlib.h>
#include <fb/protocol/internal.h>
#include <fb/acceptor.h>
#include <fb/internal/model.h>
#include <fb/internal/exception.h>
#include <fb/config.h>

namespace fb { namespace internal {

class user
{
public:
    const uint16_t                  group = 0;

public:
    user(uint16_t group) : group(group)
    { }
};

class context : public fb::base::acceptor<fb::internal::socket, fb::internal::session>
{
public:
    using service                   = fb::internal::socket<fb::internal::session>;
    using handler                   = std::function<async::task<bool>(service&)>;
    using subscriber_container      = std::map<uint8_t, service*>;
    using unique_session            = std::unique_ptr<fb::internal::session>;
    using unique_users              = std::map<std::string, std::unique_ptr<user>>;

private:
    std::map<uint8_t, handler>      _handler;
    service*                        _gateway = nullptr;
    service*                        _login   = nullptr;
    subscriber_container            _games;
    unique_users                    _users;

public:
    context(boost::asio::io_context& context, uint16_t port);
    ~context();

private:
    service*                        get(fb::protocol::internal::services type, uint8_t group = 0xFF);

protected:
    async::task<bool>               handle_parse(fb::internal::socket<fb::internal::session>& socket) final;
    fb::internal::session*          handle_accepted(fb::internal::socket<fb::internal::session>& socket) final;
    bool                            handle_connected(fb::internal::socket<fb::internal::session>& session) final;
    async::task<bool>               handle_disconnected(fb::internal::socket<fb::internal::session>& session) final;

public:
    template <typename R>
    void                            bind(const std::function<async::task<bool>(fb::internal::socket<fb::internal::session>&, R&)>& fn)
    {
        auto id = R().__id;
        _handler.insert({ id, [this, fn](fb::internal::socket<fb::internal::session>& socket)
        {
            return socket.in_stream<async::task<bool>>([this, &fn, &socket](auto& in_stream)
            {
                R       header;
                header.deserialize(in_stream);

                return fn(socket, header);
            });
        }});
    }

public:
    async::task<bool>               handle_subscribe(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::subscribe&);
    async::task<bool>               handle_transfer(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::transfer&);
    async::task<bool>               handle_login(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::login&);
    async::task<bool>               handle_logout(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::logout&);
    async::task<bool>               handle_whisper(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::whisper&);
    async::task<bool>               handle_shutdown(fb::internal::socket<fb::internal::session>&, const fb::protocol::internal::request::shutdown&);
};

} }

#endif // !__ACCEPTOR_INTERNAL_H__
