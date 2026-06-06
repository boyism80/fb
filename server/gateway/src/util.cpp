#include <fb/gateway/util.h>
#include <fb/model/model.h>

void fb::gateway::util::assert_client(const fb::protocol::gateway::request::version& request)
{
    if (request.v != 550)
        throw std::runtime_error(_TEXT(MESSAGE_CLIENT_VERSION_MISMATCH));

    if (request.nation != 0xD7)
        throw std::runtime_error(_TEXT(MESSAGE_CLIENT_NATION_INVALID));
}