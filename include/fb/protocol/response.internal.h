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

class transfer : public fb::protocol::base::header
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
    transfer() : fb::protocol::base::header(TRANSFER)
    { }
    transfer(const std::string& name, transfer_code code, uint16_t map, uint16_t x, uint16_t y, const std::string& ip, uint16_t port, uint32_t fd, services from) : fb::protocol::base::header(TRANSFER),
        name(name), code(code), map(map), x(x), y(y), ip(ip), port(port), fd(fd), from(from)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
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
    bool                    success;
    std::string             from;
    std::string             to;
    std::string             message;

public:
    whisper() : fb::protocol::base::header(WHISPER)
    { }
    whisper(bool success, const std::string& from, const std::string& to, const std::string& message) : fb::protocol::base::header(WHISPER),
        success(success), from(from), to(to), message(message)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->success)
                  .writestr_u8(this->from, false)
                  .writestr_u8(this->to, false)
                  .writestr_u8(this->message, false);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->success = in_stream.read_u8();
        this->from = in_stream.readstr_u8(false);
        this->to = in_stream.readstr_u8(false);
        this->message = in_stream.readstr_u8(false);
    }
};

class message : public fb::protocol::base::header
{
public:
    std::string             to;
    std::string             contents;
    uint8_t                 type;

public:
    message() : fb::protocol::base::header(MESSAGE)
    { }
    message(const std::string& to, const std::string& contents, uint8_t type) : fb::protocol::base::header(MESSAGE),
        to(to), contents(contents), type(type)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.writestr_u8(this->to, false)
                  .writestr_u8(this->contents, false)
                  .write_u8(this->type);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->to = in_stream.readstr_u8(false);
        this->contents = in_stream.readstr_u8(false);
        this->type = in_stream.read_u8();
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

#endif // !__PROTOCOL_RESPONSE_INTERNAL_H__