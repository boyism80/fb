#ifndef __PROTOCOL_RESPONSE_INTERNAL_H__
#define __PROTOCOL_RESPONSE_INTERNAL_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace internal { namespace response {

enum class transfer_code : uint8_t
{
    SUCCESS,
    CONNECTED,
    NOT_READY,
    UNKNOWN,
};

class transfer : public fb::protocol::internal::header
{
public:
    std::string             name;
    transfer_code           code;
    uint16_t                map;
    uint16_t                x, y;
    std::string             ip;
    uint16_t                port;
    uint32_t                fd;
    services                from;

public:
    transfer() : fb::protocol::internal::header(TRANSFER)
    { }
    transfer(const std::string& trans, const std::string& name, transfer_code code, uint16_t map, uint16_t x, uint16_t y, const std::string& ip, uint16_t port, uint32_t fd, services from) : fb::protocol::internal::header(TRANSFER, trans),
        name(name), code(code), map(map), x(x), y(y), ip(ip), port(port), fd(fd), from(from)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::internal::header::serialize(out_stream);
        out_stream.writestr_u8(this->name, false)
                  .write_u8(this->code)
                  .write_u16(this->map)
                  .write_u16(this->x)
                  .write_u16(this->y)
                  .writestr_u8(this->ip, false)
                  .write_u16(this->port)
                  .write_u32(this->fd)
                  .write_u8(this->from);
    }

    void deserialize(fb::istream& in_stream)
    {
        fb::protocol::internal::header::deserialize(in_stream);

        this->name = in_stream.readstr_u8(false);
        this->code = transfer_code(in_stream.read_u8());
        this->map = in_stream.read_16();
        this->x = in_stream.read_16();
        this->y = in_stream.read_16();
        this->ip = in_stream.readstr_u8(false);
        this->port = in_stream.read_u16();
        this->fd = in_stream.read_u32();
        this->from = (services)in_stream.read_u8();
    }
};

class logout : public fb::protocol::internal::header
{
public:
    std::string             name;

public:
    logout() : fb::protocol::internal::header(LOGOUT)
    { }
    logout(const std::string& trans, const std::string& name) : fb::protocol::internal::header(LOGOUT, trans),
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

class message : public fb::protocol::internal::header
{
public:
    std::string             to;
    std::string             contents;
    uint8_t                 type;

public:
    message() : fb::protocol::internal::header(MESSAGE)
    { }
    message(const std::string& trans, const std::string& to, const std::string& contents, uint8_t type) : fb::protocol::internal::header(MESSAGE, trans),
        to(to), contents(contents), type(type)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::internal::header::serialize(out_stream);
        out_stream.writestr_u8(this->to, false)
                  .writestr_u8(this->contents, false)
                  .write_u8(this->type);
    }

    void deserialize(fb::istream& in_stream)
    {
        fb::protocol::internal::header::deserialize(in_stream);

        this->to = in_stream.readstr_u8(false);
        this->contents = in_stream.readstr_u8(false);
        this->type = in_stream.read_u8();
    }
};

class shutdown : public fb::protocol::internal::header
{
public:
    shutdown() : fb::protocol::internal::header(SHUTDOWN)
    { }

    shutdown(const std::string& trans) : fb::protocol::internal::header(SHUTDOWN, trans)
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

#endif // !__PROTOCOL_RESPONSE_INTERNAL_H__