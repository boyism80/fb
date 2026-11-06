#include <fb/game/character.h>

#include <fb/game/server.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace fb::game;

namespace internal_reqs = fb::protocol::internal::request;

character::friends::friends(character& owner) :
    _owner(owner)
{ }

const std::vector<friend_entry>& character::friends::entries() const
{
    this->_owner.assert_thread();
    return this->_entries;
}

void character::friends::load(std::vector<friend_entry> value, uint64_t revision)
{
    this->_owner.assert_thread();
    if (revision < this->_revision)
        return;

    this->_entries       = std::move(value);
    this->_revision      = revision;
    this->_sent_revision = std::max(this->_sent_revision, revision);
}

uint64_t character::friends::next_revision()
{
    this->_owner.assert_thread();
    this->_sent_revision = std::max(this->_owner.server.ids.next(), this->_sent_revision + 1);
    return this->_sent_revision;
}

void character::friends::update(uint32_t uid, std::string_view name, bool mutual)
{
    this->_owner.assert_thread();

    for (auto& entry : this->_entries)
    {
        if (entry.uid == uid)
        {
            entry.name   = std::string(name);
            entry.mutual = mutual;
            return;
        }
    }
}

bool character::friends::is_mutual(uint32_t uid) const
{
    this->_owner.assert_thread();
    for (auto& entry : this->_entries)
    {
        if (entry.uid == uid)
            return entry.mutual;
    }
    return false;
}

bool character::friends::is_mutual(std::string_view name) const
{
    this->_owner.assert_thread();
    for (auto& entry : this->_entries)
    {
        if (entry.name == name)
            return entry.mutual;
    }
    return false;
}

bool character::friends::has_mutual() const
{
    this->_owner.assert_thread();
    for (auto& entry : this->_entries)
    {
        if (entry.mutual)
            return true;
    }
    return false;
}

async::task<void> character::friends::broadcast(std::string_view message, MESSAGE_TYPE type, bool mutual_only)
{
    this->_owner.assert_thread();

    auto message_str = std::string(message);
    auto to_uids     = std::vector<uint32_t>{};
    auto locals      = std::vector<std::shared_ptr<character>>{};
    for (auto& entry : this->_entries)
    {
        if (mutual_only && entry.mutual == false)
            continue;

        to_uids.push_back(entry.uid);
        auto ch = this->_owner.server.characters.find(entry.uid);
        if (ch != nullptr)
            locals.push_back(ch);
    }

    // Send on each friend's owning thread (same pattern as AMQP friend_message).
    if (locals.empty() == false)
    {
        this->_owner.server.characters.foreach_enqueue(
            [message_str, type](std::shared_ptr<character>& ch) -> async::task<void> {
                ch->message(message_str, type);
                co_return;
            },
            locals);
    }

    if (to_uids.empty() == false)
    {
        auto world         = this->_owner.world();
        auto host          = fb::config<uint32_t>("id");
        auto process_world = fb::config<std::optional<uint32_t>>("world");
        std::ignore        = co_await this->_owner.server.http.post("internal",
                                                             "/in-game/friend-broadcast",
                                                             internal_reqs::FriendBroadcast{world,
                                                                                            host,
                                                                                            this->_owner.id,
                                                                                            this->_owner.name(),
                                                                                            message_str,
                                                                                            static_cast<uint8_t>(type),
                                                                                            std::move(to_uids),
                                                                                            process_world});
    }
    co_return;
}

void character::friends::sync(uint8_t enabled)
{
    this->_owner.assert_thread();
    this->_owner.listener.on_friends_sync(this->_owner, enabled);
}
