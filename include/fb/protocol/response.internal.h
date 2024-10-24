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

class logout : public fb::protocol::internal::header
{
public:
    std::string name;

public:
    logout() :
        fb::protocol::internal::header(LOGOUT)
    { }
    logout(const std::string& trans, const std::string& name) :
        fb::protocol::internal::header(LOGOUT, trans),
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
    std::string to;
    std::string contents;
    uint8_t     type;

public:
    message() :
        fb::protocol::internal::header(MESSAGE)
    { }
    message(const std::string& trans, const std::string& to, const std::string& contents, uint8_t type) :
        fb::protocol::internal::header(MESSAGE, trans),
        to(to),
        contents(contents),
        type(type)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::internal::header::serialize(out_stream);
        out_stream.writestr_u8(this->to, false).writestr_u8(this->contents, false).write_u8(this->type);
    }

    void deserialize(fb::istream& in_stream)
    {
        fb::protocol::internal::header::deserialize(in_stream);

        this->to       = in_stream.readstr_u8(false);
        this->contents = in_stream.readstr_u8(false);
        this->type     = in_stream.read_u8();
    }
};

class shutdown : public fb::protocol::internal::header
{
public:
    shutdown() :
        fb::protocol::internal::header(SHUTDOWN)
    { }

    shutdown(const std::string& trans) :
        fb::protocol::internal::header(SHUTDOWN, trans)
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

}}}} // namespace fb::protocol::internal::response

#endif // !__PROTOCOL_RESPONSE_INTERNAL_H__