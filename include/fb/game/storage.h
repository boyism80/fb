#ifndef FB_GAME_STORAGE_H
#define FB_GAME_STORAGE_H

#include <async/task.h>
#include <fb/model/datetime.h>
#include <fb/model/model.h>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace fb::protocol::internal {
class StorageBox;
} // namespace fb::protocol::internal

namespace fb::game {

class character;

class storage_box
{
public:
    struct entry
    {
        uint32_t                           id = 0;
        std::optional<uint32_t>            system_storage_box_id;
        std::string                        title;
        std::string                        message;
        std::vector<fb::model::dsl>        attachments;
        bool                               received    = false;
        std::optional<fb::model::datetime> expire_date = std::nullopt;
    };
    using entry_map = std::map<uint32_t, entry>;
    using box_dto   = fb::protocol::internal::StorageBox;

    enum class receive_result
    {
        RECEIVED,
        NOT_FOUND,
        ALREADY_RECEIVED,
        EXPIRED,
        INVENTORY_FULL,
        FAILED,
    };

private:
    character& _owner;
    entry_map  _entries;
    uint32_t   _sequence = 1;

    // clang-format off
    static std::string attachments_to_json(const std::vector<fb::model::dsl>& attachments);
    static box_dto     to_save_dto(uint32_t user_id, const entry& box);
    static bool        expired(const entry& box, const fb::model::datetime& now);
    // clang-format on

public:
    // clang-format off
    static const char*              result_name(receive_result result);

    explicit                        storage_box(character& owner);
    void                            init(const std::vector<entry>& entries);
    void                            add(const std::vector<entry>& entries);
    void                            sync(const std::vector<entry>& offered, const std::unordered_set<uint32_t>& listed);
    bool                            contains_system_box(uint32_t system_storage_box_id) const;
    [[nodiscard]] async::task<receive_result> receive_reward(uint32_t entry_id);
    const entry_map&                entries() const;
    std::vector<box_dto>            to_save_dtos(uint32_t user_id, const fb::model::datetime& now) const;
    // clang-format on
};
} // namespace fb::game
#endif // FB_GAME_STORAGE_H
