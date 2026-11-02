#include <fb/bot/brain/atlas.h>

#include <fb/config.h>
#include <fb/logger.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iterator>
#include <mutex>
#include <queue>
#include <tuple>
#include <utility>

using namespace fb::bot;
using namespace fb::model;
using namespace fb::model::enum_value;

bool atlas::grid::movable(const position& at) const
{
    if (at.x >= this->width || at.y >= this->height)
        return false;

    return this->blocked[at.y * this->width + at.x] == false;
}

bool atlas::grid::passable(const position& from, DIRECTION direction) const
{
    auto to    = atlas::forward(from, direction);
    auto leave = uint8_t{0};
    auto enter = uint8_t{0};
    switch (direction)
    {
    case DIRECTION::LEFT:
        leave = fb::sobj_tbl_file::WEST;
        enter = fb::sobj_tbl_file::EAST;
        break;

    case DIRECTION::TOP:
        leave = fb::sobj_tbl_file::NORTH;
        enter = fb::sobj_tbl_file::SOUTH;
        break;

    case DIRECTION::RIGHT:
        leave = fb::sobj_tbl_file::EAST;
        enter = fb::sobj_tbl_file::WEST;
        break;

    case DIRECTION::BOTTOM:
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

uint32_t atlas::grid::region(const position& at) const
{
    if (at.x >= this->width || at.y >= this->height || this->regions.empty())
        return 0;

    return this->regions[at.y * this->width + at.x];
}

atlas::atlas(const std::string& maps_path, const fb::sobj_tbl_file& sobj) :
    _maps_path(maps_path),
    _sobj(sobj)
{
    auto warps  = table::warp;
    auto worlds = table::world;
    for (auto& [id, entries] : warps)
    {
        for (auto& entry : entries)
        {
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
                auto hop    = base;
                hop.dest    = static_cast<uint16_t>(params.id);
                hop.after   = position(params.x, params.y);
                this->_warps[static_cast<uint16_t>(id)].push_back(hop);
            }
            else if (entry.dest.header == DSL::world)
            {
                auto params = fb::model::dsl::world(entry.dest.params);
                auto nodes  = worlds->find(params.id);
                auto from   = nodes != nullptr ? nodes->find(params.index) : nullptr;
                if (from == nullptr)
                    continue;

                for (auto& [link, to] : *nodes)
                {
                    if (link == params.index || to.group != from->group)
                        continue;

                    auto hop       = base;
                    hop.dest       = static_cast<uint16_t>(to.map);
                    hop.after      = to.position;
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

    auto spawns = table::mob_spawn;
    for (auto& [id, entries] : spawns)
    {
        for (auto& entry : entries)
        {
            this->_spawns[static_cast<uint16_t>(id)].push_back(spawn{entry.mob, entry.begin, entry.end, entry.count});
        }
    }

    auto mobs = table::mob;
    for (auto& [map, entries] : this->_spawns)
    {
        this->_ranks.insert({map, this->_ranks.size()});
        auto seen = std::unordered_set<std::string>{};
        for (auto& entry : entries)
        {
            auto model = mobs->find(entry.mob);
            if (model != nullptr && seen.insert(model->name).second)
                this->_habitats[model->name].push_back(map);
        }
    }

    auto npcs     = table::npc;
    auto dwelling = table::npc_spawn;
    auto order    = size_t{0};
    for (auto& [map, entries] : dwelling)
    {
        for (auto& entry : entries)
        {
            auto model = npcs->find(entry.npc);
            if (model != nullptr)
            {
                this->_dwellers[model->name].push_back(dweller{
                    order,
                    static_cast<uint16_t>(map),
                    position{entry.position.x, entry.position.y},
                    model->name
                });
            }
            order++;
        }
    }

    fb::logger::info("brain atlas: warp_maps={} spawn_maps={}", this->_warps.size(), this->_spawns.size());
}

std::vector<uint16_t> atlas::habitats(const std::unordered_set<std::string>& names) const
{
    auto result = std::vector<uint16_t>{};
    auto added  = std::unordered_set<uint16_t>{};
    for (auto& name : names)
    {
        auto found = this->_habitats.find(name);
        if (found == this->_habitats.end())
            continue;

        for (auto map : found->second)
        {
            if (added.insert(map).second)
                result.push_back(map);
        }
    }
    std::sort(result.begin(), result.end(), [this](uint16_t left, uint16_t right) {
        return this->_ranks.at(left) < this->_ranks.at(right);
    });
    return result;
}

std::vector<atlas::dweller> atlas::dwellers(const std::unordered_set<std::string>& names) const
{
    auto result = std::vector<dweller>{};
    for (auto& name : names)
    {
        auto found = this->_dwellers.find(name);
        if (found != this->_dwellers.end())
            result.insert(result.end(), found->second.begin(), found->second.end());
    }
    std::sort(result.begin(), result.end(), [](const dweller& left, const dweller& right) {
        return left.order < right.order;
    });
    return result;
}

const atlas& atlas::get()
{
    static auto once     = std::once_flag{};
    static auto instance = std::unique_ptr<atlas>{};
    std::call_once(once, [] {
        auto sobj = fb::sobj_tbl_file{};
        sobj.load(fb::config<std::string>("brain:sobj", "../game/SObj.tbl"));
        instance = std::unique_ptr<atlas>(new atlas(fb::config<std::string>("brain:maps", "../game/maps"), sobj));
    });
    return *instance;
}

uint64_t atlas::gate(uint16_t map, const position& at)
{
    return (static_cast<uint64_t>(map) << 32) | (static_cast<uint64_t>(at.y) << 16) | at.x;
}

atlas::position atlas::forward(const position& from, DIRECTION direction)
{
    auto to = from;
    switch (direction)
    {
    case DIRECTION::LEFT:
        to.x = static_cast<uint16_t>(from.x - 1);
        break;

    case DIRECTION::TOP:
        to.y = static_cast<uint16_t>(from.y - 1);
        break;

    case DIRECTION::RIGHT:
        to.x = static_cast<uint16_t>(from.x + 1);
        break;

    case DIRECTION::BOTTOM:
        to.y = static_cast<uint16_t>(from.y + 1);
        break;
    }
    return to;
}

const atlas::grid* atlas::find_grid(uint16_t map) const
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_grids.find(map);
    if (found != this->_grids.end())
        return found->second.get();

    auto& slot = this->_grids[map];
    auto  file = std::ifstream(std::format("{}/{:06}.map", this->_maps_path, map), std::ios::binary);
    auto  bytes =
        file.is_open() ? std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {}) : std::vector<uint8_t>{};
    if (bytes.size() < 4)
        return nullptr;

    auto loaded    = std::make_unique<grid>();
    loaded->width  = static_cast<uint16_t>((bytes[0] << 8) | bytes[1]);
    loaded->height = static_cast<uint16_t>((bytes[2] << 8) | bytes[3]);
    auto size      = static_cast<size_t>(loaded->width) * loaded->height;
    if (size == 0 || bytes.size() < 4 + size * 4)
        return nullptr;

    loaded->blocked    = std::vector<bool>(size, false);
    loaded->collisions = std::vector<uint8_t>(size, 0);
    for (size_t i = 0; i < size; i++)
    {
        auto offset           = 4 + i * 4;
        auto tile             = static_cast<uint16_t>((bytes[offset] << 8) | bytes[offset + 1]);
        auto object           = static_cast<uint16_t>((bytes[offset + 2] << 8) | bytes[offset + 3]);
        loaded->blocked[i]    = tile == 0 || this->_sobj.fully_blocked(object);
        loaded->collisions[i] = this->_sobj.collision(object);
    }

    auto block = std::ifstream(std::format("{}/{:06}.block", this->_maps_path, map), std::ios::binary);
    if (block.is_open())
    {
        auto blocks = std::vector<uint8_t>(std::istreambuf_iterator<char>(block), {});
        auto count  = blocks.size() >= 4 ? blocks[0] | (blocks[1] << 8) | (blocks[2] << 16) | (blocks[3] << 24) : 0u;
        for (size_t i = 0; i < count && 4 + i * 4 + 3 < blocks.size(); i++)
        {
            auto offset = 4 + i * 4;
            auto x      = static_cast<uint16_t>(blocks[offset] | (blocks[offset + 1] << 8));
            auto y      = static_cast<uint16_t>(blocks[offset + 2] | (blocks[offset + 3] << 8));
            if (x < loaded->width && y < loaded->height)
                loaded->blocked[y * loaded->width + x] = true;
        }
    }

    static constexpr auto sides = std::array<std::pair<DIRECTION, DIRECTION>, 4>{
        std::pair{DIRECTION::LEFT,   DIRECTION::RIGHT },
        std::pair{DIRECTION::TOP,    DIRECTION::BOTTOM},
        std::pair{DIRECTION::RIGHT,  DIRECTION::LEFT  },
        std::pair{DIRECTION::BOTTOM, DIRECTION::TOP   },
    };
    auto& grid = *loaded;
    grid.regions.assign(size, 0);
    auto label = uint32_t{0};
    for (auto y = uint16_t{0}; y < grid.height; y++)
    {
        for (auto x = uint16_t{0}; x < grid.width; x++)
        {
            auto seed = position(x, y);
            if (grid.regions[y * grid.width + x] != 0 || grid.movable(seed) == false || this->warp_at(map, seed))
                continue;

            label++;
            grid.regions[y * grid.width + x] = label;
            auto pending                     = std::vector<position>{seed};
            while (pending.empty() == false)
            {
                auto at = pending.back();
                pending.pop_back();
                for (auto& [direction, back] : sides)
                {
                    auto next = atlas::forward(at, direction);
                    if (grid.movable(next) == false || this->warp_at(map, next) ||
                        grid.regions[next.y * grid.width + next.x] != 0)
                        continue;

                    if (grid.passable(at, direction) == false && grid.passable(next, back) == false)
                        continue;

                    grid.regions[next.y * grid.width + next.x] = label;
                    pending.push_back(next);
                }
            }
        }
    }

    slot = std::move(loaded);
    return slot.get();
}

const std::vector<atlas::warp>& atlas::warps(uint16_t map) const
{
    static const auto empty = std::vector<warp>{};
    auto              found = this->_warps.find(map);
    return found != this->_warps.end() ? found->second : empty;
}

const std::vector<atlas::spawn>& atlas::spawns(uint16_t map) const
{
    static const auto empty = std::vector<spawn>{};
    auto              found = this->_spawns.find(map);
    return found != this->_spawns.end() ? found->second : empty;
}

const std::unordered_map<uint16_t, std::vector<atlas::spawn>>& atlas::spawns() const
{
    return this->_spawns;
}

std::optional<uint32_t> atlas::mob(uint16_t map, uint16_t look, uint8_t color) const
{
    auto mobs = table::mob;
    for (auto& spawn : this->spawns(map))
    {
        auto model = mobs->find(spawn.mob);
        if (model != nullptr && model->look == look && model->color == color)
            return spawn.mob;
    }

    for (auto& [id, model] : mobs)
    {
        if (model.look == look && model.color == color)
            return id;
    }
    return std::nullopt;
}

bool atlas::warp_at(uint16_t map, const position& at) const
{
    auto found = this->_warp_tiles.find(map);
    return found != this->_warp_tiles.end() && found->second.contains((at.y << 16) | at.x);
}

bool atlas::reaches(uint16_t map, const position& from, const position& gate) const
{
    auto grid = this->find_grid(map);
    if (grid == nullptr)
        return true;

    auto start = grid->region(from);
    if (start == 0 || grid->region(gate) == start)
        return true;

    auto sides = std::array<std::pair<position, DIRECTION>, 4>{
        std::pair{position(gate.x + 1, gate.y),     DIRECTION::LEFT  },
        std::pair{position(gate.x,     gate.y + 1), DIRECTION::TOP   },
        std::pair{position(gate.x - 1, gate.y),     DIRECTION::RIGHT },
        std::pair{position(gate.x,     gate.y - 1), DIRECTION::BOTTOM},
    };
    return std::any_of(sides.begin(), sides.end(), [&](const auto& side) {
        return grid->region(side.first) == start && grid->passable(side.first, side.second);
    });
}

std::optional<atlas::plan> atlas::route(uint16_t                            from,
                                        const position&                     at,
                                        const std::unordered_set<uint16_t>& targets,
                                        uint8_t                             level,
                                        const std::unordered_set<uint64_t>& closed,
                                        const std::unordered_set<uint32_t>& perilous,
                                        const std::unordered_set<uint16_t>& shunned) const
{
    auto deadly = [this, &perilous, &targets](uint16_t map) {
        auto& spawns = this->spawns(map);
        return targets.contains(map) == false &&
               std::any_of(spawns.begin(), spawns.end(), [&perilous](const spawn& spawn) {
                   return perilous.contains(spawn.mob);
               });
    };

    struct visit
    {
        uint32_t    cost;
        position    at;
        const warp* via;
    };

    using node  = std::pair<uint32_t, uint16_t>;
    auto exits  = std::array{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};
    auto visits = std::unordered_map<uint16_t, visit>{
        {from, visit{0, at, nullptr}}
    };
    auto open = std::priority_queue<node, std::vector<node>, std::greater<node>>{};
    open.push({0, from});
    auto reached = std::optional<uint16_t>{};
    while (open.empty() == false)
    {
        auto [cost, current] = open.top();
        open.pop();

        auto here = visits.at(current);
        if (cost > here.cost)
            continue;

        if (targets.contains(current) && shunned.contains(current) == false)
        {
            reached = current;
            break;
        }

        for (auto& hop : this->warps(current))
        {
            if (level < hop.min_level || level > hop.max_level || closed.contains(atlas::gate(current, hop.before)) ||
                (deadly(hop.dest) || shunned.contains(hop.dest) && targets.contains(hop.dest) == false) ||
                this->reaches(current, here.at, hop.before) == false)
                continue;

            auto landing = this->find_grid(hop.dest);
            if (landing == nullptr || std::none_of(exits.begin(), exits.end(), [&](auto direction) {
                    return landing->passable(hop.after, direction);
                }))
                continue;

            auto steps = static_cast<uint32_t>(std::abs(hop.before.x - here.at.x) + std::abs(hop.before.y - here.at.y));
            auto next  = cost + steps + 1;
            auto known = visits.find(hop.dest);
            if (known != visits.end() && known->second.cost <= next)
                continue;

            visits.insert_or_assign(hop.dest, visit{next, hop.after, &hop});
            open.push({next, hop.dest});
        }
    }

    if (reached.has_value() == false)
        return std::nullopt;

    auto result = plan{reached.value(), {}, visits.at(reached.value()).cost, visits.at(reached.value()).at};
    for (auto hop = visits.at(reached.value()).via; hop != nullptr; hop = visits.at(hop->map).via)
    {
        result.hops.push_back(*hop);
    }
    std::reverse(result.hops.begin(), result.hops.end());
    return result;
}

atlas::position atlas::landing(uint16_t map)
{
    auto model = fb::model::table::map->find(map);
    if (model == nullptr || model->teleport.empty())
        return position(0, 0);

    auto& first = model->teleport.front();
    if (first.header == DSL::area)
    {
        auto params = fb::model::dsl::area(first.params);
        return position((params.left + params.right) / 2, (params.top + params.bottom) / 2);
    }
    else if (first.header == DSL::point)
    {
        auto params = fb::model::dsl::point(first.params);
        return position(params.x, params.y);
    }
    else
    {
        return position(0, 0);
    }
}

std::optional<atlas::leap> atlas::shortcut(uint16_t                            from,
                                           const position&                     at,
                                           const std::unordered_set<uint16_t>& targets,
                                           const std::optional<position>&      goal,
                                           uint8_t                             level,
                                           const std::unordered_set<uint64_t>& closed,
                                           const std::unordered_set<uint32_t>& perilous,
                                           const std::unordered_set<uint16_t>& shunned,
                                           const std::vector<teleport>&        teleports) const
{
    auto total = [&](uint16_t map, const position& start) -> std::optional<uint32_t> {
        auto plan = this->route(map, start, targets, level, closed, perilous, shunned);
        if (plan.has_value() == false)
            return std::nullopt;

        auto rest =
            goal.has_value()
                ? static_cast<uint32_t>(std::abs(plan->arrival.x - goal->x) + std::abs(plan->arrival.y - goal->y))
                : 0u;
        return plan->cost + rest;
    };

    auto maps  = fb::model::table::map;
    auto model = maps->find(from);
    auto root  = model != nullptr ? maps->find(model->root) : nullptr;
    auto walk  = total(from, at);
    auto best  = std::optional<leap>{};
    auto offer = [&](size_t index, CARDINAL side, uint16_t dest, std::optional<uint32_t> after) {
        if (after.has_value() == false)
            return;

        auto cost = teleports[index].cost + after.value();
        if ((walk.has_value() == false || cost < walk.value()) && (best.has_value() == false || cost < best->cost))
            best = leap{index, side, dest, cost};
    };

    for (auto index = size_t{0}; index < teleports.size(); index++)
    {
        auto& teleport = teleports[index];
        if (teleport.via == VIA::CARDINAL)
        {
            if (root == nullptr)
                continue;

            for (auto& [side, dsl] : root->cardinal)
            {
                if (dsl.header != DSL::map)
                    continue;

                auto params = fb::model::dsl::map(dsl.params);
                auto right  = std::max<uint16_t>(params.right, params.x);
                auto bottom = std::max<uint16_t>(params.bottom, params.y);
                auto dest   = static_cast<uint16_t>(params.id);
                offer(index, side, dest, total(dest, position((params.x + right) / 2, (params.y + bottom) / 2)));
            }
        }
        else if (teleport.via == VIA::REVIVE)
        {
            if (root == nullptr)
                continue;

            for (auto& [side, map] : root->revive)
            {
                auto dest = static_cast<uint16_t>(map);
                offer(index, side, dest, total(dest, atlas::landing(dest)));
            }
        }
        else if (teleport.via == VIA::FIXED)
        {
            for (auto dest : teleport.to)
            {
                offer(index, CARDINAL::EAST, dest, total(dest, atlas::landing(dest)));
            }
        }
        else
        {
            if (teleport.to.empty())
                continue;

            auto sum = uint64_t{0};
            auto all = true;
            for (auto dest : teleport.to)
            {
                auto after = total(dest, atlas::landing(dest));
                if (after.has_value() == false)
                {
                    all = false;
                    break;
                }
                sum += after.value();
            }
            if (all)
                offer(index, CARDINAL::EAST, teleport.to.front(), static_cast<uint32_t>(sum / teleport.to.size()));
        }
    }
    return best;
}

std::optional<std::vector<atlas::DIRECTION>>
atlas::path(uint16_t map, const position& from, const position& to, uint16_t range, const blocked_func& blocked) const
{
    auto grid = this->find_grid(map);
    if (grid == nullptr || from.x >= grid->width || from.y >= grid->height)
        return std::nullopt;

    auto distance = [&to](const position& at) {
        return static_cast<uint32_t>(std::abs(at.x - to.x) + std::abs(at.y - to.y));
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

        auto at = position(index % width, index / width);
        if (distance(at) <= range)
        {
            goal = index;
            break;
        }

        for (auto direction : {DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM})
        {
            auto next = atlas::forward(at, direction);
            if (grid->passable(at, direction) == false)
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
