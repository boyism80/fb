#ifndef __FB_PROTOCOL_INTERNAL_RESPONSE___
#define __FB_PROTOCOL_INTERNAL_RESPONSE___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/origin/internal.response.pong_generated.h>
#include <fb/protocol/flatbuffer/origin/internal.response.transfer_generated.h>
#include <fb/protocol/flatbuffer/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace internal { namespace response { 

enum class TransferResultCode : int8_t
{
    CONNECTED = fb::protocol::internal::response::origin::TransferResultCode::TransferResultCode_CONNECTED,
    SUCCESS = fb::protocol::internal::response::origin::TransferResultCode::TransferResultCode_SUCCESS
};

class Pong
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Pong;

public:

public:
    Pong()
    { }

    Pong(const Pong& x)
    { }

    Pong(const fb::protocol::internal::response::origin::Pong& raw)
    { }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::origin::CreatePong(builder);
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

    static Pong Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::origin::GetPong(bytes);
        return Pong(*raw);
    }
};

class Transfer
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Transfer;

public:
    TransferResultCode code;
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

    Transfer(TransferResultCode code, const std::string& name, const std::string& ip, uint16_t port, uint16_t map, uint16_t x, uint16_t y, uint32_t fd)
        : code(code), name(name), ip(ip), port(port), map(map), x(x), y(y), fd(fd)
    { }
    Transfer(const fb::protocol::internal::response::origin::Transfer& raw)
        : code((fb::protocol::internal::response::TransferResultCode)raw.code()), name(raw.name()->c_str()), ip(raw.ip()->c_str()), port(raw.port()), map(raw.map()), x(raw.x()), y(raw.y()), fd(raw.fd())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::origin::CreateTransfer(builder,
            (fb::protocol::internal::response::origin::TransferResultCode)this->code,
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
        auto raw = fb::protocol::internal::response::origin::GetTransfer(bytes);
        return Transfer(*raw);
    }
};

 }  }  }  } 

#endif