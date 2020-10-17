#ifndef __ACCEPTOR_INTERNAL_H__
#define __ACCEPTOR_INTERNAL_H__

#include "model/session/session.internal.h"
#include <zlib.h>
#include "protocol/internal.h"
#include "module/console/console.h"
#include "module/acceptor/acceptor.h"

namespace fb { namespace internal {

using namespace fb::protocol;

class acceptor : public fb::base::acceptor<fb::base::socket, fb::internal::session>
{
public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay);
    ~acceptor();

protected:
    void                        handle_parse(fb::base::socket<fb::internal::session>& socket);
    
public:
    internal::session*          handle_accepted(fb::base::socket<fb::internal::session>& socket);

public:
    bool                        handle_connected(fb::base::socket<fb::internal::session>& session);
    bool                        handle_disconnected(fb::base::socket<fb::internal::session>& session);
};

} }

#endif // !__ACCEPTOR_INTERNAL_H__
