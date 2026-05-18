#ifndef FB_GAME_SYSTEM_STORAGE_BOX_H
#define FB_GAME_SYSTEM_STORAGE_BOX_H

#include <cstdint>
#include <string>
#include <optional>
#include <fb/model/datetime.h>

namespace fb::protocol::internal {
class SystemStorageBox;
}

namespace fb::game {

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

    system_storage_box() = default;
    explicit system_storage_box(const fb::protocol::internal::SystemStorageBox& dto);
};

} // namespace fb::game

#endif // FB_GAME_SYSTEM_STORAGE_BOX_H
