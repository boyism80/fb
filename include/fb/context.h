#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <fb/async_local.h>
#include <fb/outbound_buffer.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>

namespace fb {

struct context
{
    std::string     transaction_id;
    outbound_buffer outbound;

    using local = async_local<context>;
};

inline std::string mint_transaction_id()
{
    static auto gen = boost::uuids::random_generator{};
    return boost::uuids::to_string(gen());
}

} // namespace fb

#endif // !__CONTEXT_H__
