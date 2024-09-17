#ifndef __FB_PROTOCOL_FLATBUFFER_INTER_RESPONSE_INTERNAL_RESPONSE_TRANSFER__
#define __FB_PROTOCOL_FLATBUFFER_INTER_RESPONSE_INTERNAL_RESPONSE_TRANSFER__

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/internal.response.transfer_generated.h>
#include <fb/protocol/flatbuffer/model/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace flatbuffer { namespace inter { namespace response { namespace model { 

class Transfer
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Transfer;

public:
    fb::protocol::flatbuffer::inter::response::TransferResultCode code;
    std::string name;
    std::string ip;
    uint16_t port;
    uint16_t map;
    uint16_t x;
    uint16_t y;
    uint32_t fd;

public:
    Transfer()
    { }

    Transfer(const Transfer& x)
        : code(x.code), name(x.name), ip(x.ip), port(x.port), map(x.map), x(x.x), y(x.y), fd(x.fd)
    { }

    Transfer(fb::protocol::flatbuffer::inter::response::TransferResultCode code, const std::string& name, const std::string& ip, uint16_t port, uint16_t map, uint16_t x, uint16_t y, uint32_t fd)
        : code(code), name(name), ip(ip), port(port), map(map), x(x), y(y), fd(fd)
    { }
    Transfer(const fb::protocol::flatbuffer::inter::response::Transfer& raw)
        : code(raw.code()), name(raw.name()->c_str()), ip(raw.ip()->c_str()), port(raw.port()), map(raw.map()), x(raw.x()), y(raw.y()), fd(raw.fd())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::flatbuffer::inter::response::CreateTransfer(builder,
            this->code,
            builder.CreateString(this->name),
            builder.CreateString(this->ip),
            this->port,
            this->map,
            this->x,
            this->y,
            this->fd);
    }
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto bytes = builder.GetBufferPointer();
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), bytes, size);
        return result;
    }

    static Transfer Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::flatbuffer::inter::response::GetTransfer(bytes);
        return Transfer(*raw);
    }
};

} } } } } }

#endif