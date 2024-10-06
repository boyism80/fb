#ifndef __FB_PROTOCOL_INTERNAL_REQUEST___
#define __FB_PROTOCOL_INTERNAL_REQUEST___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/internal.h>
#include <fb/protocol/origin/internal.request.login_generated.h>
#include <fb/protocol/origin/internal.request.logout_generated.h>
#include <fb/protocol/origin/internal.request.ping_generated.h>
#include <fb/protocol/origin/internal.request.transfer_generated.h>
#include <fb/protocol/origin/internal.request.whisper_generated.h>
#include <fb/protocol/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace internal { namespace request { 


class Login
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Login;

public:
    uint32_t uid;
    std::string name;
    uint16_t map;

public:
    Login()
    { }

    Login(const Login& x)
        : uid(x.uid), name(x.name), map(x.map)
    { }

    Login(uint32_t uid, const std::string& name, uint16_t map)
        : uid(uid), name(name), map(map)
    { }
    Login(const fb::protocol::internal::request::origin::Login& raw)
        : uid(raw.uid()), name(raw.name() != nullptr ? raw.name()->c_str() : ""), map(raw.map())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreateLogin(builder,
            this->uid,
            builder.CreateString(this->name),
            this->map);
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
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Logout;

public:
    std::string name;

public:
    Logout()
    { }

    Logout(const Logout& x)
        : name(x.name)
    { }

    Logout(const std::string& name)
        : name(name)
    { }
    Logout(const fb::protocol::internal::request::origin::Logout& raw)
        : name(raw.name() != nullptr ? raw.name()->c_str() : "")
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreateLogout(builder,
            builder.CreateString(this->name));
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
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Ping;

public:
    uint8_t id;
    std::string name;
    fb::protocol::internal::Service service;
    std::string ip;
    uint16_t port;

public:
    Ping()
    { }

    Ping(const Ping& x)
        : id(x.id), name(x.name), service(x.service), ip(x.ip), port(x.port)
    { }

    Ping(uint8_t id, const std::string& name, fb::protocol::internal::Service service, const std::string& ip, uint16_t port)
        : id(id), name(name), service(service), ip(ip), port(port)
    { }
    Ping(const fb::protocol::internal::request::origin::Ping& raw)
        : id(raw.id()), name(raw.name() != nullptr ? raw.name()->c_str() : ""), service((fb::protocol::internal::Service)raw.service()), ip(raw.ip() != nullptr ? raw.ip()->c_str() : ""), port(raw.port())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreatePing(builder,
            this->id,
            builder.CreateString(this->name),
            (fb::protocol::internal::origin::Service)this->service,
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
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Transfer;

public:
    fb::protocol::internal::Service service;
    uint8_t id;

public:
    Transfer()
    { }

    Transfer(const Transfer& x)
        : service(x.service), id(x.id)
    { }

    Transfer(fb::protocol::internal::Service service, uint8_t id)
        : service(service), id(id)
    { }
    Transfer(const fb::protocol::internal::request::origin::Transfer& raw)
        : service((fb::protocol::internal::Service)raw.service()), id(raw.id())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreateTransfer(builder,
            (fb::protocol::internal::origin::Service)this->service,
            this->id);
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
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Whisper;

public:
    std::string from;
    std::string to;
    std::string message;

public:
    Whisper()
    { }

    Whisper(const Whisper& x)
        : from(x.from), to(x.to), message(x.message)
    { }

    Whisper(const std::string& from, const std::string& to, const std::string& message)
        : from(from), to(to), message(message)
    { }
    Whisper(const fb::protocol::internal::request::origin::Whisper& raw)
        : from(raw.from() != nullptr ? raw.from()->c_str() : ""), to(raw.to() != nullptr ? raw.to()->c_str() : ""), message(raw.message() != nullptr ? raw.message()->c_str() : "")
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::origin::CreateWhisper(builder,
            builder.CreateString(this->from),
            builder.CreateString(this->to),
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