#ifndef __SYSTEM_STORAGE_SERVICE_H__
#define __SYSTEM_STORAGE_SERVICE_H__

#include <fb/game/storage.h>
#include <fb/game/system_storage_box.h>
#include <fb/model/model.h>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <cstdint>

namespace fb::protocol::internal {
class StorageBox;
class SystemStorageBox;
} // namespace fb::protocol::internal

namespace fb::game {

class server;
class character;

class system_storage_service
{
private:
    static system_storage_box from_system_storage_dto(const fb::protocol::internal::SystemStorageBox& dto);
    static storage_box::entry from_login_storage_box(const fb::protocol::internal::StorageBox& dto);
    static void prune_expired_boxes(std::vector<system_storage_box>& boxes, const fb::model::datetime& now);

    uint32_t                        _poll_offset = 0;
    std::vector<system_storage_box> _pending_boxes;

public:
    fb::game::server& server;

public:
    explicit system_storage_service(fb::game::server& server);

    async::task<void> create(uint32_t                           user_id,
                             std::string_view                   external_ref,
                             std::string_view                   title,
                             std::string_view                   message,
                             const std::vector<fb::model::dsl>& attachments);
    async::task<void> sync(character& ch);
    async::task<void> poll_and_deliver();

    void on_deliver(const std::vector<uint32_t>& user_ids, const system_storage_box& box);
    void init_character(character& ch, const std::vector<storage_box::entry>& entries);
    void init_from_login(character& ch, const std::vector<fb::protocol::internal::StorageBox>& boxes);
};

} // namespace fb::game

#endif
