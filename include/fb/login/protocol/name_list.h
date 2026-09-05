#ifndef __PROTOCOL_LOGIN_NAME_LIST_H__
#define __PROTOCOL_LOGIN_NAME_LIST_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>
#include <utility>
#include <vector>

namespace fb::protocol::login::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class name_list : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x6F;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);
};

template <>
class name_list<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x6F;
    FB_PROTOCOL_VERSION_TAGS_SINCE(CLIENT_VERSION::v651, CLIENT_VERSION::v651);

public:
#ifdef BOT
    uint8_t                                       subtype = 1;
    uint16_t                                      count   = 0;
    std::string                                   name;
    uint32_t                                      crc = 0;
    std::string                                   body;
    std::vector<std::pair<std::string, uint32_t>> names;
#else
    const uint8_t                                       subtype;
    const uint16_t                                      count;
    const std::string                                   name;
    const uint32_t                                      crc;
    const std::string                                   body;
    const std::vector<std::pair<std::string, uint32_t>> names;
#endif

public:
#ifdef BOT
    name_list() = default;
#else
    name_list(uint8_t                                       subtype,
              uint16_t                                      count,
              std::string                                   name  = {},
              uint32_t                                      crc   = 0,
              std::string                                   body  = {},
              std::vector<std::pair<std::string, uint32_t>> names = {});
#endif

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

} // namespace fb::protocol::login::response

#endif
