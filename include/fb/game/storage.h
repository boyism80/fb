#ifndef FB_GAME_STORAGE_H
#define FB_GAME_STORAGE_H
#include <fb/model/datetime.h>
#include <fb/model/model.h>
#include <async/task.h>
#include <optional>
#include <string>
#include <map>
#include <vector>

namespace fb::protocol::internal {
class StorageBox;
}

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

private:
    character& _owner;
    entry_map  _entries;
    uint32_t   _sequence = 1;

    // clang-format off
    static std::string                        attachments_to_json(const std::vector<fb::model::dsl>& attachments);
    static fb::protocol::internal::StorageBox to_save_dto(uint32_t user_id, const entry& box);
    // clang-format on

public:
    // clang-format off
    explicit                                        storage_box(character& owner);
    void                                            init(const std::vector<entry>& entries);
    void                                            apply_delivered(const std::vector<entry>& delivered);
    bool                                            contains_system_box(uint32_t system_storage_box_id) const;
    [[nodiscard]] async::task<bool>                 receive_reward(uint32_t entry_id);
    const entry_map&                                entries() const;
    uint32_t                                        next_sequence() const;
    void                                            set_sequence(uint32_t value);
    std::vector<fb::protocol::internal::StorageBox> to_save_dtos(uint32_t user_id, const fb::model::datetime& now) const;
    // clang-format on
};
} // namespace fb::game
#endif // FB_GAME_STORAGE_H
