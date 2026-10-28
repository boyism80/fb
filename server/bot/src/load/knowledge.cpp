#include <fb/bot/load/knowledge.h>

#include <fb/logger.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <fstream>
#include <functional>
#include <iterator>
#include <optional>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace fb::bot::load;

bool knowledge::grid::movable(const point<uint16_t>& position) const
{
    if (position.x >= this->width || position.y >= this->height)
        return false;

    return this->blocked[position.y * this->width + position.x] == false;
}

bool knowledge::grid::passable(const point<uint16_t>& from, DIRECTION direction) const
{
    auto to    = from;
    auto leave = uint8_t{0};
    auto enter = uint8_t{0};
    switch (direction)
    {
    case DIRECTION::LEFT:
        to.x  = static_cast<uint16_t>(from.x - 1);
        leave = fb::sobj_tbl_file::WEST;
        enter = fb::sobj_tbl_file::EAST;
        break;

    case DIRECTION::TOP:
        to.y  = static_cast<uint16_t>(from.y - 1);
        leave = fb::sobj_tbl_file::NORTH;
        enter = fb::sobj_tbl_file::SOUTH;
        break;

    case DIRECTION::RIGHT:
        to.x  = static_cast<uint16_t>(from.x + 1);
        leave = fb::sobj_tbl_file::EAST;
        enter = fb::sobj_tbl_file::WEST;
        break;

    case DIRECTION::BOTTOM:
        to.y  = static_cast<uint16_t>(from.y + 1);
        leave = fb::sobj_tbl_file::SOUTH;
        enter = fb::sobj_tbl_file::NORTH;
        break;
    }

    if (from.x >= this->width || from.y >= this->height || this->movable(to) == false)
        return false;

    if ((this->collisions[from.y * this->width + from.x] & leave) != 0)
        return false;

    return (this->collisions[to.y * this->width + to.x] & enter) == 0;
}

uint32_t knowledge::grid::region(const point<uint16_t>& position) const
{
    if (position.x >= this->width || position.y >= this->height || this->regions.empty())
        return 0;

    return this->regions[position.y * this->width + position.x];
}

knowledge::knowledge(const std::string&                  maps_path,
                     const fb::sobj_tbl_file&            sobj,
                     const std::unordered_set<uint32_t>& revive_npcs)
{
    auto maps = table::map;
    for (auto& [id, model] : maps)
    {
        auto file = std::ifstream(std::format("{}/{:06}.map", maps_path, id), std::ios::binary);
        if (file.is_open() == false)
            continue;

        auto bytes = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});
        if (bytes.size() < 4)
            continue;

        auto grid   = knowledge::grid{};
        grid.width  = static_cast<uint16_t>((bytes[0] << 8) | bytes[1]);
        grid.height = static_cast<uint16_t>((bytes[2] << 8) | bytes[3]);
        auto size   = static_cast<size_t>(grid.width) * grid.height;
        if (size == 0 || bytes.size() < 4 + size * 4)
            continue;

        grid.blocked    = std::vector<bool>(size, false);
        grid.collisions = std::vector<uint8_t>(size, 0);
        for (size_t i = 0; i < size; i++)
        {
            auto offset        = 4 + i * 4;
            auto tile          = static_cast<uint16_t>((bytes[offset] << 8) | bytes[offset + 1]);
            auto object        = static_cast<uint16_t>((bytes[offset + 2] << 8) | bytes[offset + 3]);
            grid.blocked[i]    = tile == 0 || sobj.fully_blocked(object);
            grid.collisions[i] = sobj.collision(object);
        }

        auto block = std::ifstream(std::format("{}/{:06}.block", maps_path, id), std::ios::binary);
        if (block.is_open())
        {
            auto blocks = std::vector<uint8_t>(std::istreambuf_iterator<char>(block), {});
            auto count = blocks.size() >= 4 ? blocks[0] | (blocks[1] << 8) | (blocks[2] << 16) | (blocks[3] << 24) : 0u;
            for (size_t i = 0; i < count && 4 + i * 4 + 3 < blocks.size(); i++)
            {
                auto offset = 4 + i * 4;
                auto x      = static_cast<uint16_t>(blocks[offset] | (blocks[offset + 1] << 8));
                auto y      = static_cast<uint16_t>(blocks[offset + 2] | (blocks[offset + 3] << 8));
                if (x < grid.width && y < grid.height)
                    grid.blocked[y * grid.width + x] = true;
            }
        }

        this->_grids.insert({static_cast<uint16_t>(id), std::move(grid)});
    }

    auto warps  = table::warp;
    auto worlds = table::world;
    for (auto& [id, entries] : warps)
    {
        auto source = this->_grids.find(static_cast<uint16_t>(id));
        for (auto& entry : entries)
        {
            if (source == this->_grids.end() || entry.before.x >= source->second.width ||
                entry.before.y >= source->second.height)
                continue;

            this->_warp_tiles[static_cast<uint16_t>(id)].insert((entry.before.y << 16) | entry.before.x);

            auto base = warp{static_cast<uint16_t>(id), entry.before, 0, {}};
            for (auto& condition : entry.condition)
            {
                if (condition.header != DSL::level)
                    continue;

                auto level     = fb::model::dsl::level(condition.params);
                base.min_level = std::max(base.min_level, level.min.value_or(0));
                base.max_level = std::min(base.max_level, level.max.value_or(0xFF));
            }

            if (entry.dest.header == DSL::map)
            {
                auto params = fb::model::dsl::map(entry.dest.params);
                auto grid   = this->_grids.find(static_cast<uint16_t>(params.id));
                auto after  = point<uint16_t>(params.x, params.y);
                auto exits  = std::array{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};
                if (grid == this->_grids.end() || std::none_of(exits.begin(), exits.end(), [&](auto direction) {
                        return grid->second.passable(after, direction);
                    }))
                    continue;

                auto hop  = base;
                hop.dest  = static_cast<uint16_t>(params.id);
                hop.after = point<uint16_t>(params.x, params.y);
                this->_warps[static_cast<uint16_t>(id)].push_back(hop);
            }
            else if (entry.dest.header == DSL::world)
            {
                auto params = fb::model::dsl::world(entry.dest.params);
                auto nodes  = worlds->find(params.id);
                auto from   = nodes != nullptr ? nodes->find(params.index) : nullptr;
                if (from == nullptr)
                    continue;

                for (auto link : from->links)
                {
                    auto to = nodes->find(link);
                    if (to == nullptr || this->_grids.contains(static_cast<uint16_t>(to->map)) == false)
                        continue;

                    auto hop       = base;
                    hop.dest       = static_cast<uint16_t>(to->map);
                    hop.after      = to->position;
                    hop.world      = params.id;
                    hop.world_from = params.index;
                    hop.world_to   = link;
                    this->_warps[static_cast<uint16_t>(id)].push_back(hop);
                }
            }
        }
    }

    auto incoming = std::unordered_map<uint16_t, uint32_t>{};
    auto reverse  = std::unordered_map<uint16_t, std::vector<uint16_t>>{};
    for (auto& [map, hops] : this->_warps)
    {
        for (auto& hop : hops)
        {
            incoming[hop.dest]++;
            reverse[hop.dest].push_back(map);
        }
    }

    auto hub = std::max_element(incoming.begin(), incoming.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second < rhs.second;
    });
    if (hub != incoming.end())
    {
        auto reach = [](uint16_t start, const auto& next) {
            auto visited = std::unordered_set<uint16_t>{start};
            auto queue   = std::queue<uint16_t>{};
            queue.push(start);
            while (queue.empty() == false)
            {
                auto current = queue.front();
                queue.pop();
                for (auto neighbor : next(current))
                {
                    if (visited.insert(neighbor).second)
                        queue.push(neighbor);
                }
            }
            return visited;
        };

        auto forward  = reach(hub->first, [this](uint16_t map) {
            auto dests = std::vector<uint16_t>{};
            auto found = this->_warps.find(map);
            if (found != this->_warps.end())
            {
                for (auto& hop : found->second)
                {
                    dests.push_back(hop.dest);
                }
            }
            return dests;
        });
        auto backward = reach(hub->first, [&reverse](uint16_t map) {
            auto found = reverse.find(map);
            return found != reverse.end() ? found->second : std::vector<uint16_t>{};
        });

        for (auto& [map, hops] : this->_warps)
        {
            if (forward.contains(map) == false || backward.contains(map) == false)
                continue;

            std::erase_if(hops, [&forward, &backward](const warp& hop) {
                return forward.contains(hop.dest) == false || backward.contains(hop.dest) == false;
            });
        }
    }

    auto npc_spawns = table::npc_spawn;
    for (auto& [id, entries] : npc_spawns)
    {
        for (auto& entry : entries)
        {
            auto spot = npc_spot{static_cast<uint16_t>(id), entry.npc, entry.position};
            this->_npcs[spot.map].push_back(spot);
            if (revive_npcs.contains(entry.npc) && this->_grids.contains(spot.map))
                this->_revive_spots.push_back(spot);
        }
    }

    auto promotions = table::promotion;
    for (auto& [cls, steps] : promotions)
    {
        for (auto& [step, promotion] : steps)
        {
            this->_classes.insert({promotion.name, cls});
        }
    }

    auto npcs  = table::npc;
    auto items = table::item;
    auto sells = table::sell;
    auto buys  = table::buy;
    for (auto& [id, model] : npcs)
    {
        auto entry = shop{};
        for (auto table_id : model.sell)
        {
            auto sales = sells->find(table_id);
            if (sales == nullptr)
                continue;

            for (auto& sale : *sales)
            {
                auto item = items->find(sale.item);
                if (item != nullptr)
                    entry.sells.insert({item->name, sale.price.value_or(item->price)});
            }
        }

        auto purchases = model.buy.has_value() ? buys->find(model.buy.value()) : nullptr;
        if (purchases != nullptr)
        {
            for (auto& [_, purchase] : *purchases)
            {
                auto item = items->find(purchase.item);
                if (item != nullptr)
                    entry.buys.insert({item->name, purchase.price.value_or(item->price / 2)});
            }
        }

        if (entry.sells.empty() == false || entry.buys.empty() == false)
        {
            for (auto& [name, price] : entry.buys)
            {
                this->_buyable[name] = std::max(this->_buyable[name], price);
            }

            for (auto& [name, price] : entry.sells)
            {
                auto found = this->_purchases.find(name);
                if (found == this->_purchases.end() || price < found->second)
                    this->_purchases.insert_or_assign(name, price);
            }
            this->_shops.insert({id, std::move(entry)});
        }
    }

    auto mobs       = table::mob;
    auto mob_spawns = table::mob_spawn;
    for (auto& [id, entries] : mob_spawns)
    {
        if (this->_grids.contains(static_cast<uint16_t>(id)) == false)
            continue;

        auto ground    = hunting_ground{static_cast<uint16_t>(id), {}, {}};
        auto max_count = uint16_t{0};
        for (auto& entry : entries)
        {
            auto mob = mobs->find(entry.mob);
            if (mob == nullptr || mob->invincible || mob->exp == 0 || entry.count == 0)
                continue;

            auto drops = std::unordered_map<std::string, double>{};
            auto drop  = mob->drop.empty() ? nullptr : table::drop->find(mob->drop);
            if (drop != nullptr)
            {
                for (auto& dsl : drop->dsl)
                {
                    if (dsl.header != DSL::item)
                        continue;

                    auto params = fb::model::dsl::item(dsl.params);
                    auto item   = items->find(params.id);
                    if (item != nullptr)
                    {
                        drops[item->name] += params.percent;
                        this->_droppable.insert(item->name);
                    }
                }
            }

            auto interval = std::max(0.5, mob->speed.total_milliseconds() / 1000.0);
            auto look     = static_cast<uint16_t>(mob->look + MOB_LOOK_OFFSET);
            auto found    = std::find_if(ground.preys.begin(), ground.preys.end(), [&mob](const auto& prey) {
                return prey.name == mob->name;
            });
            if (found == ground.preys.end())
            {
                ground.preys.push_back(prey{look,
                                            entry.count,
                                            static_cast<uint32_t>(mob->exp),
                                            static_cast<uint32_t>(mob->damage.max),
                                            static_cast<uint32_t>(mob->hp),
                                            {std::pair{entry.begin, entry.end}},
                                            std::move(drops),
                                            interval,
                                            mob->attack_type == fb::model::enum_value::MOB_ATTACK_TYPE::AGGRESSIVE,
                                            mob->name});
            }
            else
            {
                found->count      += entry.count;
                found->exp         = std::min(found->exp, static_cast<uint32_t>(mob->exp));
                found->damage      = std::max(found->damage, static_cast<uint32_t>(mob->damage.max));
                found->hp          = std::max(found->hp, static_cast<uint32_t>(mob->hp));
                found->aggressive |= mob->attack_type == fb::model::enum_value::MOB_ATTACK_TYPE::AGGRESSIVE;
                found->areas.push_back({entry.begin, entry.end});
            }

            if (entry.count > max_count)
            {
                max_count     = entry.count;
                ground.center = point<uint16_t>((entry.begin.x + entry.end.x) / 2, (entry.begin.y + entry.end.y) / 2);
            }
        }

        if (ground.preys.empty())
            continue;

        this->_hunting_grounds.push_back(std::move(ground));
    }

    static constexpr auto sides = std::array<std::pair<DIRECTION, DIRECTION>, 4>{
        std::pair{DIRECTION::LEFT,   DIRECTION::RIGHT },
        std::pair{DIRECTION::TOP,    DIRECTION::BOTTOM},
        std::pair{DIRECTION::RIGHT,  DIRECTION::LEFT  },
        std::pair{DIRECTION::BOTTOM, DIRECTION::TOP   },
    };
    for (auto& [map, grid] : this->_grids)
    {
        grid.regions.assign(static_cast<size_t>(grid.width) * grid.height, 0);
        auto label = uint32_t{0};
        for (auto y = uint16_t{0}; y < grid.height; y++)
        {
            for (auto x = uint16_t{0}; x < grid.width; x++)
            {
                auto seed = point<uint16_t>(x, y);
                if (grid.regions[y * grid.width + x] != 0 || grid.movable(seed) == false || this->warp_at(map, seed))
                    continue;

                label++;
                grid.regions[y * grid.width + x] = label;
                auto pending                     = std::vector<point<uint16_t>>{seed};
                while (pending.empty() == false)
                {
                    auto position = pending.back();
                    pending.pop_back();
                    for (auto& [direction, back] : sides)
                    {
                        auto next = position;
                        if (direction == DIRECTION::LEFT)
                            next.x--;
                        else if (direction == DIRECTION::TOP)
                            next.y--;
                        else if (direction == DIRECTION::RIGHT)
                            next.x++;
                        else
                            next.y++;

                        if (grid.movable(next) == false || this->warp_at(map, next) ||
                            grid.regions[next.y * grid.width + next.x] != 0)
                            continue;

                        if (grid.passable(position, direction) == false && grid.passable(next, back) == false)
                            continue;

                        grid.regions[next.y * grid.width + next.x] = label;
                        pending.push_back(next);
                    }
                }
            }
        }
    }

    fb::logger::info("load knowledge: grids={} warp_maps={} hunting_grounds={} revive_spots={}",
                     this->_grids.size(),
                     this->_warps.size(),
                     this->_hunting_grounds.size(),
                     this->_revive_spots.size());
}

const knowledge::grid* knowledge::find_grid(uint16_t map) const
{
    auto found = this->_grids.find(map);
    if (found == this->_grids.end())
        return nullptr;

    return &found->second;
}

bool knowledge::npc_at(uint16_t map, const point<uint16_t>& position) const
{
    auto found = this->_npcs.find(map);
    if (found == this->_npcs.end())
        return false;

    return std::any_of(found->second.begin(), found->second.end(), [&position](const auto& spot) {
        return spot.position == position;
    });
}

bool knowledge::warp_at(uint16_t map, const point<uint16_t>& position) const
{
    auto found = this->_warp_tiles.find(map);
    return found != this->_warp_tiles.end() && found->second.contains((position.y << 16) | position.x);
}

bool knowledge::reaches(uint16_t map, const point<uint16_t>& from, const point<uint16_t>& gate) const
{
    auto grid = this->find_grid(map);
    if (grid == nullptr)
        return true;

    auto start = grid->region(from);
    if (start == 0 || grid->region(gate) == start)
        return true;

    auto sides = std::array<std::pair<point<uint16_t>, DIRECTION>, 4>{
        std::pair{point<uint16_t>(gate.x + 1, gate.y),     DIRECTION::LEFT  },
        std::pair{point<uint16_t>(gate.x,     gate.y + 1), DIRECTION::TOP   },
        std::pair{point<uint16_t>(gate.x - 1, gate.y),     DIRECTION::RIGHT },
        std::pair{point<uint16_t>(gate.x,     gate.y - 1), DIRECTION::BOTTOM},
    };
    return std::any_of(sides.begin(), sides.end(), [&](const auto& side) {
        return grid->region(side.first) == start && grid->passable(side.first, side.second);
    });
}

std::optional<knowledge::npc_spot> knowledge::find_npc(uint16_t map, uint32_t npc) const
{
    auto found = this->_npcs.find(map);
    if (found == this->_npcs.end())
        return std::nullopt;

    auto spot = std::find_if(found->second.begin(), found->second.end(), [npc](const auto& spot) {
        return spot.npc == npc;
    });
    if (spot == found->second.end())
        return std::nullopt;

    return *spot;
}

std::vector<knowledge::npc_spot> knowledge::npc_spots(uint32_t npc) const
{
    auto spots = std::vector<npc_spot>{};
    for (auto& [_, entries] : this->_npcs)
    {
        for (auto& spot : entries)
        {
            if (spot.npc == npc && this->_grids.contains(spot.map))
                spots.push_back(spot);
        }
    }
    return spots;
}

std::optional<CLASS> knowledge::find_class(const std::string& name) const
{
    auto found = this->_classes.find(name);
    if (found == this->_classes.end())
        return std::nullopt;

    return found->second;
}

const std::unordered_map<uint32_t, knowledge::shop>& knowledge::shops() const
{
    return this->_shops;
}

const knowledge::shop* knowledge::find_shop(uint32_t npc) const
{
    auto found = this->_shops.find(npc);
    if (found == this->_shops.end())
        return nullptr;

    return &found->second;
}

bool knowledge::buyable(const std::string& item) const
{
    return this->_buyable.contains(item);
}

bool knowledge::droppable(const std::string& item) const
{
    return this->_droppable.contains(item);
}

std::optional<uint64_t> knowledge::purchase(const std::string& item) const
{
    auto found = this->_purchases.find(item);
    return found != this->_purchases.end() ? std::optional<uint64_t>{found->second} : std::nullopt;
}

uint64_t knowledge::price(const std::string& item) const
{
    auto found = this->_buyable.find(item);
    return found != this->_buyable.end() ? found->second : 0;
}

std::vector<knowledge::npc_spot> knowledge::buyers(const std::vector<std::string>& items) const
{
    auto spots = std::vector<npc_spot>{};
    for (auto& [npc, shop] : this->_shops)
    {
        auto buys = std::any_of(items.begin(), items.end(), [&shop](const auto& item) {
            return shop.buys.contains(item);
        });
        if (buys == false)
            continue;

        auto found = this->npc_spots(npc);
        spots.insert(spots.end(), found.begin(), found.end());
    }
    return spots;
}

std::vector<knowledge::npc_spot> knowledge::sellers(const std::string& item) const
{
    auto spots = std::vector<npc_spot>{};
    for (auto& [npc, shop] : this->_shops)
    {
        if (shop.sells.contains(item) == false)
            continue;

        auto found = this->npc_spots(npc);
        spots.insert(spots.end(), found.begin(), found.end());
    }
    return spots;
}

const std::vector<knowledge::hunting_ground>& knowledge::hunting_grounds() const
{
    return this->_hunting_grounds;
}

const std::vector<knowledge::npc_spot>& knowledge::revive_spots() const
{
    return this->_revive_spots;
}

uint64_t knowledge::gate_key(uint16_t map, const point<uint16_t>& position)
{
    return (static_cast<uint64_t>(map) << 32) | (static_cast<uint64_t>(position.y) << 16) | position.x;
}

std::unordered_map<uint16_t, knowledge::visit> knowledge::search(uint16_t                from,
                                                                 const point<uint16_t>&  position,
                                                                 uint8_t                 level,
                                                                 const queue_map&        queues,
                                                                 std::optional<uint16_t> until) const
{
    using node  = std::pair<uint32_t, uint16_t>;
    auto visits = std::unordered_map<uint16_t, visit>{
        {from, visit{0, position, nullptr}}
    };
    auto open = std::priority_queue<node, std::vector<node>, std::greater<node>>{};
    open.push({0, from});
    while (open.empty() == false)
    {
        auto [cost, current] = open.top();
        open.pop();

        auto here = visits.at(current);
        if (cost > here.cost)
            continue;

        if (until.has_value() && current == until.value())
            break;

        auto found = this->_warps.find(current);
        if (found == this->_warps.end())
            continue;

        for (auto& hop : found->second)
        {
            if (level < hop.min_level || level > hop.max_level || this->reaches(current, here.at, hop.before) == false)
                continue;

            auto steps = static_cast<uint32_t>(std::abs(hop.before.x - here.at.x) + std::abs(hop.before.y - here.at.y));
            auto queued = queues.find(knowledge::gate_key(current, hop.before));
            auto next   = cost + steps + 1 + (queued != queues.end() ? queued->second : 0u);
            auto known  = visits.find(hop.dest);
            if (known != visits.end() && known->second.cost <= next)
                continue;

            visits.insert_or_assign(hop.dest, visit{next, hop.after, &hop});
            open.push({next, hop.dest});
        }
    }
    return visits;
}

std::optional<std::vector<knowledge::warp>> knowledge::route(uint16_t               from,
                                                             const point<uint16_t>& position,
                                                             uint16_t               to,
                                                             uint8_t                level,
                                                             const queue_map&       queues) const
{
    if (from == to)
        return std::vector<warp>{};

    auto visits = this->search(from, position, level, queues, to);
    if (visits.contains(to) == false)
        return std::nullopt;

    auto hops = std::vector<warp>{};
    for (auto hop = visits.at(to).via; hop != nullptr; hop = visits.at(hop->map).via)
    {
        hops.push_back(*hop);
    }
    std::reverse(hops.begin(), hops.end());
    return hops;
}

std::unordered_map<uint16_t, uint32_t> knowledge::distances(uint16_t               from,
                                                            const point<uint16_t>& position,
                                                            uint8_t                level) const
{
    auto costs = std::unordered_map<uint16_t, uint32_t>{};
    for (auto& [map, visit] : this->search(from, position, level))
    {
        costs.insert({map, visit.cost});
    }
    return costs;
}

std::optional<std::vector<DIRECTION>> knowledge::path(uint16_t               map,
                                                      const point<uint16_t>& from,
                                                      const point<uint16_t>& to,
                                                      uint16_t               range,
                                                      const blocked_func&    blocked) const
{
    auto grid = this->find_grid(map);
    if (grid == nullptr || from.x >= grid->width || from.y >= grid->height)
        return std::nullopt;

    auto distance = [&to](const point<uint16_t>& position) {
        return static_cast<uint32_t>(std::abs(position.x - to.x) + std::abs(position.y - to.y));
    };

    using node = std::pair<uint32_t, uint32_t>;
    auto width = static_cast<uint32_t>(grid->width);
    auto start = from.y * width + from.x;
    auto cost  = std::unordered_map<uint32_t, uint32_t>{
        {start, 0}
    };
    auto previous = std::unordered_map<uint32_t, std::pair<uint32_t, DIRECTION>>{};
    auto open     = std::priority_queue<node, std::vector<node>, std::greater<node>>{};
    open.push({distance(from), start});

    auto goal     = std::optional<uint32_t>{};
    auto searched = uint32_t{0};
    auto limit    = static_cast<uint32_t>(grid->width) * grid->height * 2;
    while (open.empty() == false && searched++ < limit)
    {
        auto [_, index] = open.top();
        open.pop();

        auto position = point<uint16_t>(index % width, index / width);
        if (distance(position) <= range)
        {
            goal = index;
            break;
        }

        static constexpr auto steps = std::array<std::tuple<int, int, DIRECTION>, 4>{
            std::tuple{-1, 0,  DIRECTION::LEFT  },
            std::tuple{0,  -1, DIRECTION::TOP   },
            std::tuple{1,  0,  DIRECTION::RIGHT },
            std::tuple{0,  1,  DIRECTION::BOTTOM},
        };
        for (auto& [dx, dy, direction] : steps)
        {
            auto next = point<uint16_t>(position.x + dx, position.y + dy);
            if (grid->passable(position, direction) == false)
                continue;

            if (distance(next) > range && blocked(next))
                continue;

            if (next != to && this->warp_at(map, next))
                continue;

            auto next_index = next.y * width + next.x;
            auto next_cost  = cost.at(index) + 1;
            auto found      = cost.find(next_index);
            if (found != cost.end() && found->second <= next_cost)
                continue;

            cost.insert_or_assign(next_index, next_cost);
            previous.insert_or_assign(next_index, std::pair{index, direction});
            open.push({next_cost + distance(next), next_index});
        }
    }

    if (goal.has_value() == false)
        return std::nullopt;

    auto directions = std::vector<DIRECTION>{};
    for (auto index = goal.value(); index != start; index = previous.at(index).first)
    {
        directions.push_back(previous.at(index).second);
    }
    std::reverse(directions.begin(), directions.end());
    return directions;
}
