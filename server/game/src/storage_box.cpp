#include <fb/game/storage.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/model/datetime.h>
#include <fb/model/model.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <algorithm>

using namespace fb::game;

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

        auto id   = this->_sequence++;
        auto e    = entry{};
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
        mark.user                   = this->_owner.id;
        mark.pending_id             = box.id;
        mark.expire_date            = box.expire_date;
        this->_reward_marks[box.id] = mark;

        // Log storage box entry addition
        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(this->_owner.id);
        log_data["character_name"] = UTF8(this->_owner.name(), PLATFORM::WINDOWS);
        log_data["entry_id"]       = static_cast<Json::Int64>(id);
        log_data["pending_id"]     = box.id;
        log_data["title"]          = UTF8(box.title, PLATFORM::WINDOWS);
        auto attachments_array     = Json::Value(Json::arrayValue);
        for (const auto& attachment : e.attachments)
        {
            attachments_array.append(attachment.to_json());
        }
        log_data["attachments"] = attachments_array;
        log_data["expire_date"] = e.expire_date.has_value()
                                      ? Json::Value(UTF8(e.expire_date.value().to_string(), PLATFORM::WINDOWS))
                                      : Json::Value::null;
        this->_owner.server.log.write("storage_box_entry_add", log_data);

        // Notify player about new storage box entry
        if (!box.title.empty())
        {
            this->_owner.message(std::format(_TEXT(MESSAGE_STORAGE_BOX_REWARD_ADDED), box.title), MESSAGE_TYPE::STATE);
        }
        else
        {
            this->_owner.message(_TEXT(MESSAGE_STORAGE_BOX_REWARD_ADDED_NO_TITLE), MESSAGE_TYPE::STATE);
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

    auto now = this->_owner.server.now();
    if (it->second.expire_date.has_value() && it->second.expire_date.value() < now)
        return false;

    if (this->_owner.reward(it->second.attachments) == false)
        return false;

    // Log storage box attachment reception
    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->_owner.id);
    log_data["character_name"] = UTF8(this->_owner.name(), PLATFORM::WINDOWS);
    log_data["entry_id"]       = static_cast<Json::Int64>(entry_id);
    log_data["title"]          = UTF8(it->second.title, PLATFORM::WINDOWS);
    auto attachments_array     = Json::Value(Json::arrayValue);
    for (const auto& attachment : it->second.attachments)
    {
        attachments_array.append(attachment.to_json());
    }
    log_data["attachments"] = attachments_array;
    this->_owner.server.log.write("storage_box_attachment_receive", log_data);

    it->second.received = true;
    return true;
}

const storage_box::entry_map& storage_box::entries() const
{
    this->_owner.assert_thread();
    return this->_entries;
}

const storage_box::reward_mark_map& storage_box::reward_marks() const
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