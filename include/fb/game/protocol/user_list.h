#ifndef __PROTOCOL_GAME_USER_LIST_H__
#define __PROTOCOL_GAME_USER_LIST_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/socket.h>
#ifndef BOT
#include <fb/game/character.h>
using namespace fb::game;
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class user_list : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x18;

public:
    user_list() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

class user_list : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x36;

private:
#ifdef BOT
#else
    using container = fb::locker<std::unordered_map<uint32_t, std::unique_ptr<fb::socket<character>>>>;
#endif

public:
#ifdef BOT
#else
    const character& me;
    container&       sockets;
#endif

public:
#ifdef BOT
    user_list() = default;
#else
    user_list(const character& me, container& sockets);
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif