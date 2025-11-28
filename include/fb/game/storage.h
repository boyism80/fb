#ifndef FB_GAME_STORAGE_H
#define FB_GAME_STORAGE_H

#include <fb/model/datetime.h>
#include <fb/model/model.h>
#include <optional>
#include <string>
#include <unordered_map>
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
        std::string                        title;
        std::string                        message;
        std::vector<fb::model::dsl>        attachments;
        bool                               received    = false;
        std::optional<fb::model::datetime> expire_date = std::nullopt;
    };

    struct reward_mark
    {
        uint32_t                           user = 0;
        std::string                        pending_id;
        std::optional<fb::model::datetime> expire_date = std::nullopt;
    };

    struct pending_box
    {
        std::string                        id;
        std::optional<uint32_t>            user = std::nullopt;
        std::string                        title;
        std::string                        message;
        std::vector<fb::model::dsl>        attachments;
        std::optional<fb::model::datetime> expire_date = std::nullopt;
    };

private:
    character&                                   _owner;
    std::map<uint32_t, entry>                    _entries;
    std::unordered_map<std::string, reward_mark> _reward_marks;
    uint32_t                                     _sequence = 1;

public:
    explicit storage_box(character& owner);

    void                                             init(const std::vector<entry>& entries, const std::vector<reward_mark>& marks);
    void                                             apply_pending(const std::vector<pending_box>& pending);
    bool                                             receive_reward(uint32_t entry_id);
    const std::map<uint32_t, entry>&                 entries() const;
    const std::unordered_map<uint64_t, reward_mark>& reward_marks() const;
    uint32_t                                         next_sequence() const;
    void                                             set_sequence(uint32_t value);
};

} // namespace fb::game

#endif // FB_GAME_STORAGE_H
