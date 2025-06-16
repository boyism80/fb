#ifndef __SERVICE_GATEWAY_H__
#define __SERVICE_GATEWAY_H__

#include <fb/gateway/protocol.h>

namespace fb::gateway::util {

/**
 * @brief      Validates client version compatibility with the gateway server.
 *
 *             Checks if the client version provided in the request is compatible
 *             with the current server version. Throws an exception if the client
 *             version is outdated or incompatible.
 *
 * @param[in]  request  The version request containing client version information.
 *
 * @throws     std::runtime_error if the client version is incompatible.
 */
void assert_client(const fb::protocol::gateway::request::version& request);

} // namespace fb::gateway::util
#endif // !__SERVICE_GATEWAY_H__
