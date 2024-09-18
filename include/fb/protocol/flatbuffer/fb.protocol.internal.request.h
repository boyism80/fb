#ifndef __FB_PROTOCOL_INTERNAL_REQUEST___
#define __FB_PROTOCOL_INTERNAL_REQUEST___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/fb.protocol.internal.h>
#include <fb/protocol/flatbuffer/origin/internal.request.login_generated.h>
#include <fb/protocol/flatbuffer/origin/internal.request.logout_generated.h>
#include <fb/protocol/flatbuffer/origin/internal.request.ping_generated.h>
#include <fb/protocol/flatbuffer/origin/internal.request.transfer_generated.h>
#include <fb/protocol/flatbuffer/origin/internal.request.whisper_generated.h>
#include <fb/protocol/flatbuffer/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace internal { namespace request { 


class Login
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Login;

public:
    uint32_t uid;

public:
    Login()
    { }

    Login(const Login& x)
        : uid(x.uid)
    { }

    Login(uint32_t uid)
        : uid(uid)
    { }
    Login(const fb::protocol::internal::request::origin::Login& raw)
        : uid(raw.uid())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreateLogin(builder,
            this->uid);
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

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::origin::GetLogin(bytes);
        return Login(*raw);
    }
};

class Logout
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Logout;

public:
    uint32_t uid;

public:
    Logout()
    { }

    Logout(const Logout& x)
        : uid(x.uid)
    { }

    Logout(uint32_t uid)
        : uid(uid)
    { }
    Logout(const fb::protocol::internal::request::origin::Logout& raw)
        : uid(raw.uid())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreateLogout(builder,
            this->uid);
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

    static Logout Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::origin::GetLogout(bytes);
        return Logout(*raw);
    }
};

class Ping
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Ping;

public:
    std::string name;
    fb::protocol::internal::Service service;
    uint8_t group;
    std::string ip;
    uint16_t port;

public:
    Ping()
    { }

    Ping(const Ping& x)
        : name(x.name), service(x.service), group(x.group), ip(x.ip), port(x.port)
    { }

    Ping(const std::string& name, fb::protocol::internal::Service service, uint8_t group, const std::string& ip, uint16_t port)
        : name(name), service(service), group(group), ip(ip), port(port)
    { }
    Ping(const fb::protocol::internal::request::origin::Ping& raw)
        : name(raw.name()->c_str()), service((fb::protocol::internal::Service)raw.service()), group(raw.group()), ip(raw.ip()->c_str()), port(raw.port())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreatePing(builder,
            builder.CreateString(this->name),
            (fb::protocol::internal::origin::Service)this->service,
            this->group,
            builder.CreateString(this->ip),
            this->port);
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
    uint32_t id;
    fb::protocol::internal::Service service;
    uint8_t group;

public:
    Transfer()
    { }

    Transfer(const Transfer& x)
        : id(x.id), service(x.service), group(x.group)
    { }

    Transfer(uint32_t id, fb::protocol::internal::Service service, uint8_t group)
        : id(id), service(service), group(group)
    { }
    Transfer(const fb::protocol::internal::request::origin::Transfer& raw)
        : id(raw.id()), service((fb::protocol::internal::Service)raw.service()), group(raw.group())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreateTransfer(builder,
            this->id,
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