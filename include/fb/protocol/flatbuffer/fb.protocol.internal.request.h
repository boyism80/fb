#ifndef __FB_PROTOCOL_INTERNAL_REQUEST___
#define __FB_PROTOCOL_INTERNAL_REQUEST___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/fb.protocol.internal.h>
#include <fb/protocol/flatbuffer/origin/internal.request.ping_generated.h>
#include <fb/protocol/flatbuffer/origin/internal.request.transfer_generated.h>
#include <fb/protocol/flatbuffer/origin/internal.request.whisper_generated.h>
#include <fb/protocol/flatbuffer/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace internal { namespace request { 


class Ping
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Ping;

public:
    std::string name;
    fb::protocol::internal::Service service;
    uint8_t group;

public:
    Ping()
    { }

    Ping(const Ping& x)
        : name(x.name), service(x.service), group(x.group)
    { }

    Ping(const std::string& name, fb::protocol::internal::Service service, uint8_t group)
        : name(name), service(service), group(group)
    { }
    Ping(const fb::protocol::internal::request::origin::Ping& raw)
        : name(raw.name()->c_str()), service((fb::protocol::internal::Service)raw.service()), group(raw.group())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreatePing(builder,
            builder.CreateString(this->name),
            (fb::protocol::internal::origin::Service)this->service,
            this->group);
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

    static Ping Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::origin::GetPing(bytes);
        return Ping(*raw);
    }
};

class Transfer
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Transfer;

public:
    std::string name;
    fb::protocol::internal::Service from;
    fb::protocol::internal::Service to;
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

    Transfer(const std::string& name, fb::protocol::internal::Service from, fb::protocol::internal::Service to, uint16_t map, uint16_t x, uint16_t y, uint32_t fd)
        : name(name), from(from), to(to), map(map), x(x), y(y), fd(fd)
    { }
    Transfer(const fb::protocol::internal::request::origin::Transfer& raw)
        : name(raw.name()->c_str()), from((fb::protocol::internal::Service)raw.from()), to((fb::protocol::internal::Service)raw.to()), map(raw.map()), x(raw.x()), y(raw.y()), fd(raw.fd())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreateTransfer(builder,
            builder.CreateString(this->name),
            (fb::protocol::internal::origin::Service)this->from,
            (fb::protocol::internal::origin::Service)this->to,
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
        auto raw = fb::protocol::internal::request::origin::GetTransfer(bytes);
        return Transfer(*raw);
    }
};

class Whisper
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Whisper;

public:
    std::string name;
    std::string message;

public:
    Whisper()
    { }

    Whisper(const Whisper& x)
        : name(x.name), message(x.message)
    { }

    Whisper(const std::string& name, const std::string& message)
        : name(name), message(message)
    { }
    Whisper(const fb::protocol::internal::request::origin::Whisper& raw)
        : name(raw.name()->c_str()), message(raw.message()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreateWhisper(builder,
            builder.CreateString(this->name),
            builder.CreateString(this->message));
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

    static Whisper Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::origin::GetWhisper(bytes);
        return Whisper(*raw);
    }
};

 }  }  }  } 

#endif