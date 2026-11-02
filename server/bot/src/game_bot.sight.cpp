#include <fb/bot/game_bot.h>

#include <cstdint>
#include <iterator>
#include <mutex>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace fb::bot;

constexpr size_t EVENT_CAPACITY = 512;

void game_bot::sight::show(uint32_t               oid,
                           const point<uint16_t>& position,
                           uint16_t               look,
                           uint8_t                color,
                           std::string_view       name)
{
    auto found   = this->_entries.find(oid);
    auto percent = found != this->_entries.end() ? found->second.percent : uint8_t{100};
    this->_entries.insert_or_assign(oid, entry{oid, position, look, color, std::string(name), percent});
    this->_dead.erase(oid);
}

void game_bot::sight::move(uint32_t oid, const point<uint16_t>& position)
{
    auto found = this->_entries.find(oid);
    if (found == this->_entries.end())
        return;

    found->second.position = position;
}

void game_bot::sight::hurt(uint32_t oid, uint8_t percent)
{
    auto found = this->_entries.find(oid);
    if (found == this->_entries.end())
        return;

    found->second.percent = percent;
}

void game_bot::sight::hide(uint32_t oid)
{
    this->_entries.erase(oid);
}

void game_bot::sight::die(uint32_t oid)
{
    this->_entries.erase(oid);
    this->_dead.insert(oid);
}

void game_bot::sight::clear()
{
    this->_entries.clear();
    this->_dead.clear();
}

bool game_bot::sight::dead(uint32_t oid) const
{
    return this->_dead.contains(oid);
}

const std::unordered_map<uint32_t, game_bot::sight::entry>& game_bot::sight::entries() const
{
    return this->_entries;
}

void game_bot::events::push(entry value)
{
    auto lock = std::lock_guard(this->_mutex);
    if (this->_entries.size() >= EVENT_CAPACITY)
        this->_entries.pop_front();

    this->_entries.push_back(std::move(value));
}

std::vector<game_bot::events::entry> game_bot::events::take()
{
    auto lock    = std::lock_guard(this->_mutex);
    auto entries = std::vector<entry>(std::make_move_iterator(this->_entries.begin()),
                                      std::make_move_iterator(this->_entries.end()));
    this->_entries.clear();
    return entries;
}
