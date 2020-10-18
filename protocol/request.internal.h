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

public:
    subscribe() {}
    subscribe(const std::string& name) : name(name) {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(id)
                  .writestr_u8(this->name);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
    }
};

class login : public fb::protocol::base::header
{
public:
    BIND_ID(LOGIN)

public:
    std::string             name;

public:
    login() {}
    login(const std::string& name) : name(name) {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(id)
                  .writestr_u8(this->name);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
    }
};

class logout : public fb::protocol::base::header
{
public:
    BIND_ID(LOGIN)

public:
    std::string             name;

public:
    logout() {}
    logout(const std::string& name) : name(name) {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(id)
                  .writestr_u8(this->name);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
    }
};

} } } }

#endif // !__PROTOCOL_REQUEST_INTERNAL_H__