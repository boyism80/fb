#include <fb/game/group.h>
#include <fb/game/context.h>

using namespace fb::game;

group::group(context& context, uint32_t id) :
    _context(context),
    _id(id)
{ }

group::group(group&& g) :
    _context(g._context),
    _id(g._id),
    _master(g._master),
    _members(std::move(g._members)),
    _active_members(std::move(g._active_members))
{ }

void group::enter(character& ch)
{
    auto i = std::find(this->_active_members.begin(), this->_active_members.end(), &ch);
    if (i != this->_active_members.end())
        return;

    this->_active_members.push_back(&ch);
}

void group::leave(character& ch)
{
    auto i = std::find(this->_active_members.begin(), _active_members.end(), &ch);
    if (i == this->_active_members.end())
        return;

    this->_active_members.erase(i);
}

async::task<void> group::update(const std::string& master, const std::vector<std::string>& members)
{
    this->_master = master;

    this->_members.clear();
    for (auto& member : members)
    {
        this->_members.push_back(member);
    }

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

    for (auto& [thread, names] : g)
    {
        if (names.size() == 0)
            continue;

        co_await thread->switching();
        auto params = thread->template data<thread_params>();
        for (auto& name : names)
        {
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

bool group::inited() const
{
    return !this->_master.empty();
}

const std::string& group::master() const
{
    return this->_master;
}

std::vector<character*> group::characters() const
{
    return this->_active_members;
}

std::vector<std::string> group::members() const
{
    return std::vector<std::string>(this->_members);
}

std::vector<character*> group::nears(const fb::game::map& map, const point16_t& position) const
{
    auto nears  = map.nears(position, OBJECT_TYPE::CHARACTER); // same thread
    auto result = std::vector<character*>();

    for (auto ch : nears)
    {
        auto i = std::find(this->_active_members.begin(), this->_active_members.end(), ch);
        if (i != this->_active_members.end())
            result.push_back(*i);
    }

    return result;
}

fb::thread* group::thread() const
{
    return this->_context.threads.modular(this->_id);
}