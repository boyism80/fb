#ifndef __FB_GAME_SERVICE_SYSTEM_STORAGE_H__
#define __FB_GAME_SERVICE_SYSTEM_STORAGE_H__

#include <fb/game/storage.h>
#include <fb/game/system_storage_box.h>
#include <fb/model/model.h>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace fb::protocol::internal {
class StorageBox;
class SystemStorageBox;
class StorageWriteEntry;
} // namespace fb::protocol::internal

namespace fb::game {
class server;
class character;
} // namespace fb::game

namespace fb::game::service {

class system_storage
{
private:
    static system_storage_box from_system_storage_dto(const fb::protocol::internal::SystemStorageBox& dto);
    static storage_box::entry from_storage_box_dto(const fb::protocol::internal::StorageBox& dto);
    static void        prune_expired_boxes(std::vector<system_storage_box>& boxes, const fb::model::datetime& now);
    static std::string attachments_to_json(const std::vector<fb::model::dsl>& attachments);

    uint32_t                        _poll_offset = 0;
    std::vector<system_storage_box> _pending_boxes;

public:
    fb::game::server& server;

public:
    explicit system_storage(fb::game::server& server);

public:
    // clang-format off
    async::task<bool> create(uint32_t user_id, std::string_view external_ref, std::string_view title, std::string_view message, const std::vector<fb::model::dsl>& attachments, const std::optional<std::string>& expire_date = std::nullopt);
    async::task<bool> create(std::string_view user_name, std::string_view title, std::string_view message, const std::vector<fb::model::dsl>& attachments, const std::optional<std::string>& expire_date = std::nullopt, std::string_view external_ref = "");
    async::task<bool> create_system(std::string_view title, std::string_view message, const std::vector<fb::model::dsl>& attachments, const std::optional<std::string>& expire_date = std::nullopt, std::string_view external_ref = "");
    async::task<void> sync(character& ch);
    async::task<void> poll_and_deliver();
    void              deliver(const std::vector<storage_box::entry>& entries, const std::vector<uint32_t>& user_ids);
    void              on_write_box(const fb::protocol::internal::StorageBox& dto);
    void              on_deliver(const std::vector<fb::protocol::internal::StorageWriteEntry>& entries);
    void              init_character(character& ch, const std::vector<storage_box::entry>& entries);
    void              init_from_login(character& ch, const std::vector<fb::protocol::internal::StorageBox>& boxes);
    // clang-format on
};

} // namespace fb::game::service

#endif
