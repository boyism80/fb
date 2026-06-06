#ifndef FB_GAME_MARRIAGE_H
#define FB_GAME_MARRIAGE_H

#include <fb/model/datetime.h>
#include <optional>
#include <string>
#include <utility>
#include <fb/protocol/flatbuffer/protocol.h>

namespace fb::game {

struct marriage
{
    std::optional<uint32_t> spouse_id;
    std::string             spouse_name;
    fb::model::datetime     remarriage_after;
    uint32_t                divorce_count;

    explicit marriage(const fb::model::datetime& remarriage_after,
                      std::optional<uint32_t>    spouse_id     = std::nullopt,
                      std::string                spouse_name   = "",
                      uint32_t                   divorce_count = 0) :
        spouse_id(spouse_id),
        spouse_name(std::move(spouse_name)),
        remarriage_after(remarriage_after),
        divorce_count(divorce_count)
    { }

    marriage() = delete;

    fb::protocol::internal::Marriage to_protocol() const;
};

} // namespace fb::game

#endif // FB_GAME_MARRIAGE_H
