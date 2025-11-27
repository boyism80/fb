#include <fb/game/storage.h>
#include <fb/game/character.h>
#include <fb/model/datetime.h>
#include <fb/model/model.h>
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
        this->_entries.emplace(e.id, e);
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

        auto  id = this->_sequence++;
        entry e{};
        e.id      = id;
        e.title   = box.title;
        e.message = box.message;
        e.attachments.reserve(box.attachments.size());
        for (const auto& attachment : box.attachments)
        {
            e.attachments.emplace_back(attachment);
        }
        e.received    = false;
        e.expire_date = box.expire_date;

        this->_entries.emplace(id, e);
        reward_mark mark{};
        mark.user                   = this->_owner.id();
        mark.pending_id             = box.id;
        mark.expire_date            = box.expire_date;
        this->_reward_marks[box.id] = mark;

        // Notify player about new storage box entry
        if (!box.title.empty())
        {
            this->_owner.message(std::format("통합보관함에 '{}' 보상이 추가되었습니다.", box.title), MESSAGE_TYPE::STATE);
        }
        else
        {
            this->_owner.message("통합보관함에 보상이 추가되었습니다.", MESSAGE_TYPE::STATE);
        }
    }
}

bool storage_box::receive_reward(uint32_t entry_id)
{
    this->_owner.assert_thread();

    auto it = this->_entries.find(entry_id);
    if (it == this->_entries.end())
        return false;

    if (it->second.received)
        return false;

    auto now = fb::model::datetime();
    if (it->second.expire_date.has_value() && it->second.expire_date.value() < now)
        return false;

    if (this->_owner.reward(it->second.attachments) == false)
        return false;

    it->second.received = true;
    return true;
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
