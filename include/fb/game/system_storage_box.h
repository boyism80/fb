#ifndef FB_GAME_SYSTEM_STORAGE_BOX_H
#define FB_GAME_SYSTEM_STORAGE_BOX_H

#include <cstdint>
#include <optional>
#include <string>
#include <fb/game/storage.h>
#include <fb/model/datetime.h>

namespace fb::game {

class character;

struct system_storage_box
{
    uint32_t                           id;
    std::optional<uint32_t>            user;
    std::string                        title;
    std::string                        message;
    std::string                        attachments;
    std::optional<fb::model::datetime> expire_date;
    std::optional<std::string>         external_ref;
    fb::model::datetime                created_date;

    [[nodiscard]] bool               expired(const fb::model::datetime& now) const;
    [[nodiscard]] bool               eligible_for(const character& ch) const;
    [[nodiscard]] storage_box::entry to_entry() const;
};

} // namespace fb::game

#endif // FB_GAME_SYSTEM_STORAGE_BOX_H
