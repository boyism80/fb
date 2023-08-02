#ifndef __PROTOCOL_REQUEST_INTERNAL_H__
#define __PROTOCOL_REQUEST_INTERNAL_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace internal { namespace request {

class subscribe : public fb::protocol::base::header
{
public:
    std::string             name;
    services                service;
    uint8_t                 group;

public:
    subscribe() : fb::protocol::base::header(SUBSCRIBE)
    { }
    subscribe(const std::string& name, services service, uint8_t group) : fb::protocol::base::header(SUBSCRIBE),
        name(name), service(service), group(group)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.writestr_u8(this->name, false)
                  .write_u8(this->service)
                  .write_u8(this->group);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
        this->service = (services)in_stream.read_u8();
        this->group = in_stream.read_u8();
    }
};

class transfer : public fb::protocol::base::header
{
public:
    std::string             name;
    services                from;
    services                to;
    uint16_t                map;
    uint16_t                x, y;
    uint32_t                fd;

public:
    transfer() : fb::protocol::base::header(TRANSFER)
    { }
    transfer(const std::string& name, services from, services to, uint32_t map, uint32_t fd) : fb::protocol::base::header(TRANSFER),
        name(name), from(from), to(to), fd(fd), map(map), x(0xFFFF), y(0xFFFF)
    { }
    transfer(const std::string& name, services from, services to, uint32_t map, uint16_t x, uint16_t y, uint32_t fd) : fb::protocol::base::header(TRANSFER),
        name(name), from(from), to(to), fd(fd), map(map), x(x), y(y)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.writestr_u8(this->name, false)
                  .write_u8(this->from)
                  .write_u8(this->to)
                  .write_u16(this->map)
                  .write_u16(this->x)
                  .write_u16(this->y)
                  .write_u32(this->fd);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8(false);
        this->from = (services)in_stream.read_u8();
        this->to = (services)in_stream.read_u8();
        this->map = in_stream.read_u16();
        this->x = in_stream.read_u16();
        this->y = in_stream.read_u16();
        this->fd = in_stream.read_u32();
    }
};

class login : public fb::protocol::base::header
{
public:
    std::string             name;
    uint16_t                map;

public:
    login() : fb::protocol::base::header(LOGIN)
    { }
    login(const std::string& name, uint32_t map) : fb::protocol::base::header(LOGIN),
        name(name), map(map)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.writestr_u8(this->name, false)
                  .write_u16(this->map);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8(false);
        this->map = in_stream.read_u16();
    }
};

class logout : public fb::protocol::base::header
{
public:
    std::string             name;

public:
    logout() : fb::protocol::base::header(LOGOUT)
    { }
    logout(const std::string& name) : fb::protocol::base::header(LOGOUT),
        name(name)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.writestr_u8(this->name, false);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8(false);
    }
};

class whisper : public fb::protocol::base::header
{
public:
    std::string             from;
    std::string             to;
    std::string             message;

public:
    whisper() : fb::protocol::base::header(WHISPER)
    { }
    whisper(const std::string& from, const std::string& to, const std::string& message) : fb::protocol::base::header(WHISPER),
        from(from), to(to), message(message) 
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.writestr_u8(this->from, false)
                  .writestr_u8(this->to, false)
                  .writestr_u8(this->message, false);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->from = in_stream.readstr_u8(false);
        this->to = in_stream.readstr_u8(false);
        this->message = in_stream.readstr_u8(false);
    }
};

class shutdown : public fb::protocol::base::header
{
public:
    shutdown() : fb::protocol::base::header(SHUTDOWN)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    { 
        base::header::serialize(out_stream);
    }

    void deserialize(fb::istream& in_stream)
    { }
};

} } } }

#endif // !__PROTOCOL_REQUEST_INTERNAL_H__