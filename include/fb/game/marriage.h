#ifndef FB_GAME_MARRIAGE_H
#define FB_GAME_MARRIAGE_H

#include <fb/model/datetime.h>
#include <optional>
#include <string>

namespace fb::protocol::internal {
struct Marriage;
}

namespace fb::game {

/**
 * In-game marriage state (DAO). Holds spouse id, display name, remarriage cooldown, and divorce count.
 * Convert from protocol Marriage (DTO) on Init; convert to protocol Marriage when building Save request.
 * remarriage_after is not nullable: "can remarry" is simply (now >= remarriage_after).
 * Init with empty string -> set to now; DTO saves "" when now >= remarriage_after.
 */
struct marriage
{
    std::optional<uint32_t>  spouse_id        = std::nullopt;
    std::string              spouse_name     = "";
    fb::model::datetime      remarriage_after = fb::model::datetime();
    uint32_t                 divorce_count   = 0;

    /**
     * Converts this DAO to protocol Marriage (DTO) for Save request.
     */
    fb::protocol::internal::Marriage to_protocol() const;
};

} // namespace fb::game

#endif // FB_GAME_MARRIAGE_H
