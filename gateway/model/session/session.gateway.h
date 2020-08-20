#ifndef __SESSION_GATEWAY_H__
#define __SESSION_GATEWAY_H__

#include <module/socket/socket.h>

namespace fb { namespace gateway {

class session : public fb::base
{
private:
    fb::socket*             _socket;

public:
    std::string             created_id;

public:
    session(fb::socket* socket);
    ~session();

    // override
public:
    void                    send(const fb::ostream& stream, bool encrypt = true, bool wrap = true, bool async = true);

    // middle man
public:
    fb::cryptor&            crt();
    void                    crt(const fb::cryptor& crt);
    void                    crt(uint8_t enctype, const uint8_t* enckey);
    fb::istream&            in_stream();

public:
    operator                fb::socket& ();
};

} }

#endif // !__SESSION_GATEWAY_H__