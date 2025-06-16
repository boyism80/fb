#ifndef __HOOK_PARAMS_H__
#define __HOOK_PARAMS_H__

#include <functional>
#include <fb/protocol/header.h>

namespace fb::bot {

/**
 * @brief      Parameters for protocol message hooks in bot communication.
 *
 *             This class encapsulates the configuration for temporary hooks
 *             that intercept specific protocol messages. Hooks are used to
 *             implement request-response patterns where a bot sends a message
 *             and waits for a matching response based on custom conditions.
 *
 *             The hook system allows bots to handle asynchronous protocol
 *             communication by setting up conditional callbacks that are
 *             triggered when matching responses arrive.
 */
class hook_params
{
public:
    /**
     * @brief      Condition function to determine if a protocol message matches this hook.
     *
     *             This function is called for each incoming protocol message of the
     *             registered type. It should return true if the message matches the
     *             criteria for this specific hook (e.g., matching sequence numbers,
     *             specific field values, etc.).
     */
    std::function<bool(const fb::protocol::header&)> condition;

    /**
     * @brief      Callback function executed when the condition is matched.
     *
     *             This function is called when an incoming protocol message
     *             satisfies the condition function. It typically resolves a
     *             promise or performs the action that was waiting for this
     *             specific response. After execution, the hook is automatically
     *             removed from the hook collection.
     */
    std::function<void(const fb::protocol::header&)> matched;
};

} // namespace fb::bot

#endif