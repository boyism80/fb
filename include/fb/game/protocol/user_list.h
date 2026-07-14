#ifndef __PROTOCOL_GAME_USER_LIST_H__
#define __PROTOCOL_GAME_USER_LIST_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/socket.h>
#include <string>
#include <vector>

namespace fb::protocol::game::request {
#ifndef BOT
using namespace fb::game;
#endif
using namespace fb::model::enum_value;

class user_list : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x18;

public:
    user_list() = default;

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {
#ifndef BOT
using namespace fb::game;
#endif

class user_list : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x36;

public:
    struct user_data
    {
        uint8_t     nation;
        uint8_t     cls;
        uint8_t     promotion;
        uint8_t     level;
        uint8_t     color;
        std::string name;
    };

    std::vector<user_data> users;

public:
#ifdef BOT
    user_list() = default;
#else
    user_list(std::vector<user_data>&& users);
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
