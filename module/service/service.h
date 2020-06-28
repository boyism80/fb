#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <module/stream/stream.h>
#include <module/crypto/cryptor.h>

namespace fb {

class service
{
protected:
    service();
    ~service();

protected:
    static fb::ostream          make_transfer_stream(const fb::cryptor& crt, uint32_t ip, uint16_t port, const fb::ostream* parameter = nullptr);
};

}

#endif // !__SERVICE_H__
