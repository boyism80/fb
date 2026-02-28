#ifndef FB_GAME_MARRIAGE_H
#define FB_GAME_MARRIAGE_H

#include <fb/model/datetime.h>
#include <optional>
#include <string>
#include <fb/protocol/flatbuffer/protocol.h>

namespace fb::game {

struct marriage
{
    std::optional<uint32_t> spouse_id        = std::nullopt;
    std::string             spouse_name      = "";
    fb::model::datetime     remarriage_after = fb::model::datetime();
    uint32_t                divorce_count    = 0;

    fb::protocol::internal::Marriage to_protocol() const;
};

} // namespace fb::game

#endif // FB_GAME_MARRIAGE_H
