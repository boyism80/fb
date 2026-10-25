#ifndef __FB_GAME_SERVICE_SYSTEM_STORAGE_H__
#define __FB_GAME_SERVICE_SYSTEM_STORAGE_H__

#include <fb/async_generator.h>
#include <fb/game/storage.h>
#include <fb/game/system_storage_box.h>
#include <fb/model/model.h>

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace fb::protocol::internal {
class StorageBox;
class SystemStorageBox;
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
    fb::async_generator<void> distribute_coroutine();

    std::vector<system_storage_box> _pending_boxes;
    std::mutex                      _joined_mutex;
    std::unordered_set<uint32_t>    _joined;

public:
    fb::game::server& server;

private:
    fb::async_generator<void> _distribution;

public:
    explicit system_storage(fb::game::server& server);

public:
    // clang-format off
    async::task<bool> create(uint32_t world, std::string_view user_name, std::string_view external_ref, std::string_view title, std::string_view message, const std::vector<fb::model::dsl>& attachments, const std::optional<std::string>& expire_date = std::nullopt);
    async::task<void> sync(character& ch);
    async::task<void> distribute();
    void              join(uint32_t user);
    void              on_write(const fb::protocol::internal::SystemStorageBox& dto);
    void              init_from_login(character& ch, const std::vector<fb::protocol::internal::StorageBox>& boxes);
    // clang-format on
};

} // namespace fb::game::service

#endif
