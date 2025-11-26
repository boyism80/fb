#include <fb/game/storage.h>
#include <fb/game/character.h>
#include <algorithm>

namespace fb::game {

storage_box::storage_box(character& owner) :
    _owner(owner)
{ }

void storage_box::init(const std::vector<entry>& entries, const std::vector<reward_mark>& marks)
{
    this->_owner.assert_thread();

    this->_entries.clear();
    this->_reward_marks.clear();
    this->_sequence = 1;

    for (const auto& e : entries)
    {
        this->_entries[e.id] = e;
        if (e.id >= this->_sequence)
            this->_sequence = e.id + 1;
    }

    for (const auto& mark : marks)
    {
        this->_reward_marks[mark.pending_id] = mark;
    }
}

void storage_box::apply_pending(const std::vector<pending_box>& pending)
{
    this->_owner.assert_thread();

    for (const auto& box : pending)
    {
        if (this->_reward_marks.find(box.id) != this->_reward_marks.end())
            continue;

        entry e{};
        e.id          = this->_sequence++;
        e.message     = box.message;
        e.attachments = box.attachments;
        e.received    = false;
        e.expire_date = box.expire_date;

        this->_entries[e.id] = e;
        reward_mark mark{};
        mark.pending_id                      = box.id;
        mark.expire_date                     = box.expire_date;
        this->_reward_marks[mark.pending_id] = mark;
    }
}

const std::map<uint32_t, storage_box::entry>& storage_box::entries() const
{
    this->_owner.assert_thread();
    return this->_entries;
}

const std::unordered_map<uint64_t, storage_box::reward_mark>& storage_box::reward_marks() const
{
    this->_owner.assert_thread();
    return this->_reward_marks;
}

uint32_t storage_box::next_sequence() const
{
    this->_owner.assert_thread();
    return this->_sequence;
}

void storage_box::set_sequence(uint32_t value)
{
    this->_owner.assert_thread();
    this->_sequence = value;
}

} // namespace fb::game
