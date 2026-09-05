#ifndef __PROTOCOL_LOGIN_META_DAT_H__
#define __PROTOCOL_LOGIN_META_DAT_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class meta_dat : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x7B;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v550);

public:
#ifdef BOT
    const std::string name;
#else
    std::string name;
#endif

public:
#ifdef BOT
    explicit meta_dat(const std::string& name);
#else
    meta_dat() = default;
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::request

#endif
