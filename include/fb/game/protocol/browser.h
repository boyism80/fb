#ifndef __PROTOCOL_GAME_BROWSER_H__
#define __PROTOCOL_GAME_BROWSER_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class browser : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x73;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
#ifdef BOT
    const uint8_t type;
#else
    uint8_t type = 0;
#endif

public:
#ifdef BOT
    explicit browser(uint8_t type = 0);
#else
    browser() = default;
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class browser : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x62;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
    browser() = default;

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

template <>
class browser<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x62;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifndef BOT
    const uint8_t     type;
    const std::string url;
    const std::string key;
    const std::string cookie;
#else
    uint8_t     type = 0;
    std::string url;
    std::string key;
    std::string cookie;
#endif

public:
#ifdef BOT
    browser() = default;
#else
    browser(uint8_t type = 0, std::string url = {}, std::string key = {}, std::string cookie = {});
#endif

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

} // namespace fb::protocol::game::response

#endif
