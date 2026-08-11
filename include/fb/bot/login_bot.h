#ifndef __BOT_LOGIN_H__
#define __BOT_LOGIN_H__

#include <fb/bot/bot.h>
#include <fb/login/protocol.h>
#include <fb/protocol/client_version.h>
#include <random.h>
#include <fb/encoding.h>
#include <shared_mutex>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace fb::bot {

class login_bot_controller;
template <typename ControllerType> class bot;

class login_bot : public bot<login_bot>
{
private:
    uint8_t                      _transfer_from  = 0;
    fb::protocol::CLIENT_VERSION _client_version = fb::protocol::CLIENT_VERSION::v550;

public:
    using bot_controller_type = login_bot_controller;

public:
    login_bot(bot_controller<login_bot>& bot_controller, uint32_t id);
    login_bot(bot_controller<login_bot>& bot_controller, uint32_t id, const fb::stream& params);
    ~login_bot();

public:
    std::string                  generate_id() const;
    uint8_t                      transfer_from() const;
    fb::protocol::CLIENT_VERSION client_version() const;
};

} // namespace fb::bot

#endif
