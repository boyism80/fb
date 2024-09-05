#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fb/core/stream.h>

#define BIND_ID(n) static constexpr uint8_t id = (n);

namespace fb { namespace protocol { namespace internal {

enum class services : uint8_t
{
    GATEWAY, 
    LOGIN, 
    GAME
};

enum id : uint8_t
{
    SUBSCRIBE,
    TRANSFER,
    LOGIN,
    LOGOUT,
    WHISPER,
    MESSAGE,
    SHUTDOWN,
};

} } }

namespace fb { namespace protocol { namespace base {

class header
{
public:
    const uint32_t __id;

protected:
    header(uint32_t id) : __id(id)
    { }

public:
    ~header() = default;

public:
    virtual void            serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(this->__id);
    }
    virtual void            deserialize(fb::istream& in_stream) { }
};

} } }

namespace fb { namespace protocol { namespace internal {

class header : public fb::protocol::base::header
{
public:
    std::string trans;

protected:
    header(uint32_t id) : fb::protocol::base::header(id), trans(boost::uuids::to_string(boost::uuids::random_generator()()))
    { }
    header(uint32_t id, const std::string& trans) : fb::protocol::base::header(id), trans(trans)
    { }
    ~header() = default;

public:
    virtual void            serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.writestr_u16(this->trans);
    }

    virtual void            deserialize(fb::istream& in_stream)
    {
        fb::protocol::base::header::deserialize(in_stream);
        this->trans = in_stream.readstr_u16();
    }
};

} } }

#include <fb/protocol/request.common.h>
#include <fb/protocol/response.common.h>

#endif // !__PROTOCOL_H__
