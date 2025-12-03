#include <fb/game/group.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/protocol/flatbuffer/protocol.h>

using namespace fb::game;
using namespace fb::protocol::internal::request;

group::group(server& server, uint32_t id, const std::string& master, const std::vector<std::string>& members) :
    _server(server),
    _id(id),
    _master(master),
    _members(members)
{ }

group::group(group&& g) :
    _server(g._server),
    _id(g._id),
    _master(g._master),
    _members(std::move(g._members)),
    _active_members(std::move(g._active_members))
{ }

void group::enter(std::weak_ptr<character> ch)
{
    auto ptr = ch.lock();
    if (ptr == nullptr)
        return;

    // Prevent duplicate entries
    if (this->_active_members.contains(ptr))
        return;

    this->_active_members.insert(ptr);
}

void group::detach(std::weak_ptr<character> ch)
{
    auto ptr = ch.lock();
    if (ptr == nullptr)
        return;

    if (this->_active_members.contains(ptr) == false)
        return;

    this->_active_members.erase(ptr);
}

async::task<void> group::update(const std::string& master, const std::vector<std::string>& members)
{
    this->_master = master;

    this->_members.clear();
    for (auto& member : members)
    {
        this->_members.push_back(member);
    }

    // Group members by thread for efficient processing
    auto g = std::unordered_map<fb::thread*, std::vector<std::string>>();

    auto concated = std::vector<std::string>(members);
    concated.push_back(master);
    for (auto& name : concated)
    {
        auto hash   = std::hash<std::string>{}(name);
        auto thread = this->_server.threads.modular(hash);

        if (g.contains(thread) == false)
            g.insert({thread, std::vector<std::string>{}});

        g[thread].push_back(name);
    }

    // Process each thread's members
    for (auto& [thread, names] : g)
    {
        if (names.size() == 0)
            continue;

        co_await thread->switching();
        auto params = thread->template data<thread_params>();
        for (auto& name : names)
        {
            // TODO: Character synchronization logic needs implementation
            // if (params->characters.contains(name) == false)
            //     continue;

            // ... 다시 생각해볼 필요가 있음;;
        }
    }
}

uint32_t group::id() const
{
    return this->_id;
}

const std::string& group::master() const
{
    return this->_master;
}

std::unordered_set<std::shared_ptr<character>> group::characters() const
{
    return this->_active_members;
}

std::vector<std::string> group::members() const
{
    return std::vector<std::string>(this->_members);
}

std::vector<std::weak_ptr<character>> group::nears(const fb::game::map& map, const fb::model::point16_t& position) const
{
    // Find all characters near the position
    auto nears  = map.nears(position, OBJECT_TYPE::CHARACTER); // same thread
    auto result = std::vector<std::weak_ptr<character>>();

    // Filter only group members
    for (auto& obj : nears)
    {
        auto ch = std::static_pointer_cast<fb::game::character>(obj);
        if (this->_active_members.contains(ch))
            result.push_back(ch);
    }

    return result;
}

async::task<void> group::enter_member(character& inviter, const std::string& target_name)
{
    auto   weak = inviter.weak_from_this_as<character>();
    auto&& resp = co_await this->_server.http.post("internal", "/group/enter", EnterGroup{fb::config<uint32_t>("host"), inviter.id(), target_name});
    co_await this->_server.threads.switching(weak);
    co_await this->_server.on_updated_group(resp);
}

async::task<void> group::leave_member(character& leaver)
{
    auto   weak = leaver.weak_from_this_as<character>();
    auto&& resp = co_await this->_server.http.post("internal", "/group/leave", LeaveGroup{fb::config<uint32_t>("host"), leaver.name()});
    co_await this->_server.threads.switching(weak);
    co_await this->_server.on_updated_group(resp);
}

async::task<void> group::kick_member(character& kicker, const std::string& target_name)
{
    auto   weak = kicker.weak_from_this_as<character>();
    auto&& resp = co_await this->_server.http.post("internal", "/group/kick", KickGroup{fb::config<uint32_t>("host"), kicker.name(), target_name});
    co_await this->_server.threads.switching(weak);
    co_await this->_server.on_updated_group(resp);
}

async::task<void> group::broadcast(const std::string& message, MESSAGE_TYPE type)
{
    auto&& resp = co_await this->_server.http.post("internal", "/group/broadcast", BroadcastGroup{fb::config<uint32_t>("host"), this->_id, message, static_cast<uint8_t>(type)});
    co_await this->_server.on_group_broadcast(resp);
}