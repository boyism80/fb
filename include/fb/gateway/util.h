#ifndef __SERVICE_GATEWAY_H__
#define __SERVICE_GATEWAY_H__

#include <fb/gateway/protocol.h>

namespace fb::gateway::util {

/**
 * @brief      { function_description }
 *
 * @param[in]  request  The request
 */
void assert_client(const fb::protocol::gateway::request::version& request);

} // namespace fb::gateway::util
#endif // !__SERVICE_GATEWAY_H__
