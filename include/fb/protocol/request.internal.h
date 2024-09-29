#ifndef __PROTOCOL_REQUEST_INTERNAL_H__
#define __PROTOCOL_REQUEST_INTERNAL_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace internal { namespace request {

class subscribe : public fb::protocol::internal::header
{
public:
    std::string             name;
    services                service;
    uint8_t                 group;

public:
    subscribe() : fb::protocol::internal::header(SUBSCRIBE)
    { }
    subscribe(const std::string& name, services service, uint8_t group) : fb::protocol::internal::header(SUBSCRIBE),
        name(name), service(service), group(group)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::internal::header::serialize(out_stream);
        out_stream.writestr_u8(this->name, false)
                  .write_u8(this->service)
                  .write_u8(this->group);
    }

    void deserialize(fb::istream& in_stream)
    {
        fb::protocol::internal::header::deserialize(in_stream);

        this->name = in_stream.readstr_u8();
        this->service = (services)in_stream.read_u8();
        this->group = in_stream.read_u8();
    }
};

class login : public fb::protocol::internal::header
{
public:
    std::string             name;
    uint16_t                map;

public:
    login() : fb::protocol::internal::header(LOGIN)
    { }
    login(const std::string& name, uint32_t map) : fb::protocol::internal::header(LOGIN),
        name(name), map(map)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::internal::header::serialize(out_stream);
        out_stream.writestr_u8(this->name, false)
                  .write_u16(this->map);
    }

    void deserialize(fb::istream& in_stream)
    {
        fb::protocol::internal::header::deserialize(in_stream);

        this->name = in_stream.readstr_u8(false);
        this->map = in_stream.read_u16();
    }
};

class logout : public fb::protocol::internal::header
{
public:
    std::string             name;

public:
    logout() : fb::protocol::internal::header(LOGOUT)
    { }
    logout(const std::string& name) : fb::protocol::internal::header(LOGOUT),
        name(name)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::internal::header::serialize(out_stream);
        out_stream.writestr_u8(this->name, false);
    }

    void deserialize(fb::istream& in_stream)
    {
        fb::protocol::internal::header::deserialize(in_stream);

        this->name = in_stream.readstr_u8(false);
    }
};

class whisper : public fb::protocol::internal::header
{
public:
    std::string             from;
    std::string             to;
    std::string             message;

public:
    whisper() : fb::protocol::internal::header(WHISPER)
    { }
    whisper(const std::string& from, const std::string& to, const std::string& message) : fb::protocol::internal::header(WHISPER),
        from(from), to(to), message(message) 
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::internal::header::serialize(out_stream);
        out_stream.writestr_u8(this->from, false)
                  .writestr_u8(this->to, false)
                  .writestr_u8(this->message, false);
    }

    void deserialize(fb::istream& in_stream)
    {
        fb::protocol::internal::header::deserialize(in_stream);

        this->from = in_stream.readstr_u8(false);
        this->to = in_stream.readstr_u8(false);
        this->message = in_stream.readstr_u8(false);
    }
};

class shutdown : public fb::protocol::internal::header
{
public:
    shutdown() : fb::protocol::internal::header(SHUTDOWN)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    { 
        fb::protocol::internal::header::serialize(out_stream);
    }

    void deserialize(fb::istream& in_stream)
    {
        fb::protocol::internal::header::deserialize(in_stream);
    }
};

} } } }

#endif // !__PROTOCOL_REQUEST_INTERNAL_H__