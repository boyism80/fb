#include <fb/game/group.h>
#include <fb/game/context.h>

using namespace fb::game;

group::group(context& context, uint32_t id, const std::string& master, const std::vector<std::string>& members) :
    _context(context),
    _id(id),
    _master(master),
    _members(members)
{ }

group::group(group&& g) :
    _context(g._context),
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

void group::leave(std::weak_ptr<character> ch)
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
        auto thread = this->_context.threads.modular(hash);

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

fb::thread* group::thread() const
{
    return this->_context.threads.modular(this->_id);
}