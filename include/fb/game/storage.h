#ifndef FB_GAME_STORAGE_H
#define FB_GAME_STORAGE_H
#include <fb/model/datetime.h>
#include <fb/model/model.h>
#include <optional>
#include <string>
#include <map>
#include <vector>

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

public:
    explicit storage_box(character& owner);
    void             init(const std::vector<entry>& entries);
    void             apply_delivered(const std::vector<entry>& delivered);
    bool             contains_system_box(uint32_t system_storage_box_id) const;
    bool             receive_reward(uint32_t entry_id);
    const entry_map& entries() const;
    uint32_t         next_sequence() const;
    void             set_sequence(uint32_t value);
};
} // namespace fb::game
#endif // FB_GAME_STORAGE_H
