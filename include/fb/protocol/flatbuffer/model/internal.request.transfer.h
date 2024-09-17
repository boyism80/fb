#ifndef __FB_PROTOCOL_FLATBUFFER_INTER_REQUEST_INTERNAL_REQUEST_TRANSFER__
#define __FB_PROTOCOL_FLATBUFFER_INTER_REQUEST_INTERNAL_REQUEST_TRANSFER__

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/internal.request.transfer_generated.h>
#include <fb/protocol/flatbuffer/model/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace flatbuffer { namespace inter { namespace request { namespace model { 

class Transfer
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Transfer;

public:
    std::string name;
    fb::protocol::flatbuffer::inter::request::Service from;
    fb::protocol::flatbuffer::inter::request::Service to;
    uint16_t map;
    uint16_t x;
    uint16_t y;
    uint32_t fd;

public:
    Transfer()
    { }

    Transfer(const Transfer& x)
        : name(x.name), from(x.from), to(x.to), map(x.map), x(x.x), y(x.y), fd(x.fd)
    { }

    Transfer(const std::string& name, fb::protocol::flatbuffer::inter::request::Service from, fb::protocol::flatbuffer::inter::request::Service to, uint16_t map, uint16_t x, uint16_t y, uint32_t fd)
        : name(name), from(from), to(to), map(map), x(x), y(y), fd(fd)
    { }
    Transfer(const fb::protocol::flatbuffer::inter::request::Transfer& raw)
        : name(raw.name()->c_str()), from(raw.from()), to(raw.to()), map(raw.map()), x(raw.x()), y(raw.y()), fd(raw.fd())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::flatbuffer::inter::request::CreateTransfer(builder,
            builder.CreateString(this->name),
            this->from,
            this->to,
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
        auto raw = fb::protocol::flatbuffer::inter::request::GetTransfer(bytes);
        return Transfer(*raw);
    }
};

} } } } } }

#endif