#ifndef __PROTOCOL_REQUEST_INTERNAL_H__
#define __PROTOCOL_REQUEST_INTERNAL_H__

#include "protocol/protocol.h"

namespace fb { namespace protocol { namespace internal { namespace request {

class subscribe : public fb::protocol::base::header
{
public:
    BIND_ID(SUBSCRIBE)

public:
    std::string             name;
    services                service;
    uint8_t                group;

public:
    subscribe() = default;
    subscribe(const std::string& name, services service, uint8_t group) : 
        name(name), service(service), group(group)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(id)
                  .writestr_u8(this->name, false)
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
    BIND_ID(TRANSFER)

public:
    std::string             name;
    services                service;
    uint16_t                map;
    uint16_t                x, y;
    uint32_t                fd;

public:
    transfer() {}
    transfer(const std::string& name, services service, uint32_t map, uint32_t fd) : 
        name(name), service(service), fd(fd), map(map), x(0xFFFF), y(0xFFFF)
    {}
    transfer(const std::string& name, services service, uint32_t map, uint16_t x, uint16_t y, uint32_t fd) : 
        name(name), service(service), fd(fd), map(map), x(x), y(y)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(id)
                  .writestr_u8(this->name, false)
                  .write_u8(this->service)
                  .write_u16(this->map)
                  .write_u16(this->x)
                  .write_u16(this->y)
                  .write_u32(this->fd);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8(false);
        this->service = (services)in_stream.read_u8();
        this->map = in_stream.read_u16();
        this->x = in_stream.read_u16();
        this->y = in_stream.read_u16();
        this->fd = in_stream.read_u32();
    }
};

class login : public fb::protocol::base::header
{
public:
    BIND_ID(LOGIN)

public:
    std::string             name;
    uint16_t                map;

public:
    login() {}
    login(const std::string& name, uint32_t map) : 
        name(name), map(map)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(id)
                  .writestr_u8(this->name, false)
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
    BIND_ID(LOGOUT)

public:
    std::string             name;

public:
    logout() {}
    logout(const std::string& name) : name(name) {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(id)
                  .writestr_u8(this->name, false);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8(false);
    }
};

class whisper : public fb::protocol::base::header
{
public:
    BIND_ID(WHISPER)

public:
    std::string             from;
    std::string             to;
    std::string             message;

public:
    whisper() {}
    whisper(const std::string& from, const std::string& to, const std::string& message) : 
        from(from), to(to), message(message) 
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(id)
                  .writestr_u8(this->from, false)
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

} } } }

#endif // !__PROTOCOL_REQUEST_INTERNAL_H__