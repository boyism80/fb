#ifndef __FB_PROTOCOL_INTERNAL_RESPONSE___
#define __FB_PROTOCOL_INTERNAL_RESPONSE___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/internal.h>
#include <fb/protocol/origin/internal.response.kick_out_generated.h>
#include <fb/protocol/origin/internal.response.login_generated.h>
#include <fb/protocol/origin/internal.response.logout_generated.h>
#include <fb/protocol/origin/internal.response.pong_generated.h>
#include <fb/protocol/origin/internal.response.transfer_generated.h>
#include <fb/protocol/origin/internal.response.whisper_generated.h>
#include <fb/protocol/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace internal { namespace response { 


class KickOut
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::KickOut;

public:
    uint32_t uid;

public:
    KickOut()
    { }

    KickOut(const KickOut& x)
        : uid(x.uid)
    { }

    KickOut(uint32_t uid)
        : uid(uid)
    { }
    KickOut(const fb::protocol::internal::response::origin::KickOut& raw)
        : uid(raw.uid())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::origin::CreateKickOut(builder,
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

    static KickOut Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::origin::GetKickOut(bytes);
        return KickOut(*raw);
    }
};

class Login
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Login;

public:
    bool success;
    bool logon;
    std::string ip;
    uint16_t port;

public:
    Login()
    { }

    Login(const Login& x)
        : success(x.success), logon(x.logon), ip(x.ip), port(x.port)
    { }

    Login(bool success, bool logon, const std::string& ip, uint16_t port)
        : success(success), logon(logon), ip(ip), port(port)
    { }
    Login(const fb::protocol::internal::response::origin::Login& raw)
        : success(raw.success()), logon(raw.logon()), ip(raw.ip()->c_str()), port(raw.port())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::origin::CreateLogin(builder,
            this->success,
            this->logon,
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

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::origin::GetLogin(bytes);
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
    Logout(const fb::protocol::internal::response::origin::Logout& raw)
        : uid(raw.uid())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::origin::CreateLogout(builder,
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
        auto raw = fb::protocol::internal::response::origin::GetLogout(bytes);
        return Logout(*raw);
    }
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
    fb::protocol::internal::TransferResult code;
    std::string ip;
    uint16_t port;

public:
    Transfer()
    { }

    Transfer(const Transfer& x)
        : code(x.code), ip(x.ip), port(x.port)
    { }

    Transfer(fb::protocol::internal::TransferResult code, const std::string& ip, uint16_t port)
        : code(code), ip(ip), port(port)
    { }
    Transfer(const fb::protocol::internal::response::origin::Transfer& raw)
        : code((fb::protocol::internal::TransferResult)raw.code()), ip(raw.ip()->c_str()), port(raw.port())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::origin::CreateTransfer(builder,
            (fb::protocol::internal::origin::TransferResult)this->code,
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

    static Transfer Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::origin::GetTransfer(bytes);
        return Transfer(*raw);
    }
};

class Whisper
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Whisper;

public:
    bool success;
    std::string from;
    uint32_t to;
    std::string message;

public:
    Whisper()
    { }

    Whisper(const Whisper& x)
        : success(x.success), from(x.from), to(x.to), message(x.message)
    { }

    Whisper(bool success, const std::string& from, uint32_t to, const std::string& message)
        : success(success), from(from), to(to), message(message)
    { }
    Whisper(const fb::protocol::internal::response::origin::Whisper& raw)
        : success(raw.success()), from(raw.from()->c_str()), to(raw.to()), message(raw.message()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::origin::CreateWhisper(builder,
            this->success,
            builder.CreateString(this->from),
            this->to,
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
        auto raw = fb::protocol::internal::response::origin::GetWhisper(bytes);
        return Whisper(*raw);
    }
};

 }  }  }  } 

#endif