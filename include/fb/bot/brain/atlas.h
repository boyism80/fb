#ifndef __FB_BOT_BRAIN_ATLAS_H__
#define __FB_BOT_BRAIN_ATLAS_H__

#include <fb/model/model.h>
#include <fb/sobj_tbl_file.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fb::bot {

class atlas
{
public:
    using DIRECTION = fb::model::enum_value::DIRECTION;
    using CARDINAL  = fb::model::enum_value::CARDINAL_DIRECTION;
    using position  = fb::model::point<uint16_t>;

    static constexpr uint16_t NO_WORLD = 0xFFFF;

    struct grid
    {
        uint16_t              width  = 0;
        uint16_t              height = 0;
        std::vector<bool>     blocked;
        std::vector<uint8_t>  collisions;
        std::vector<uint32_t> regions;

        bool     movable(const position& at) const;
        bool     passable(const position& from, DIRECTION direction) const;
        uint32_t region(const position& at) const;
    };

    struct warp
    {
        uint16_t map;
        position before;
        uint16_t dest;
        position after;
        uint8_t  min_level  = 0;
        uint8_t  max_level  = 0xFF;
        uint16_t world      = NO_WORLD;
        uint16_t world_from = 0;
        uint16_t world_to   = 0;
    };

    struct spawn
    {
        uint32_t mob = 0;
        position begin;
        position end;
        uint16_t count = 0;
    };

    struct dweller
    {
        size_t      order = 0;
        uint16_t    map   = 0;
        position    at;
        std::string name;
    };

    struct plan
    {
        uint16_t          dest = 0;
        std::vector<warp> hops;
        uint32_t          cost = 0;
        position          arrival;
    };

    enum class VIA : uint8_t
    {
        CARDINAL = 0,
        REVIVE   = 1,
        FIXED    = 2,
        RANDOM   = 3,
    };

    struct teleport
    {
        VIA                   via = VIA::FIXED;
        std::vector<uint16_t> to;
        uint32_t              cost = 0;
    };

    struct leap
    {
        size_t   index = 0;
        CARDINAL side  = CARDINAL::EAST;
        uint16_t dest  = 0;
        uint32_t cost  = 0;
    };

    using blocked_func = std::function<bool(const position&)>;

private:
    std::string                                                 _maps_path;
    fb::sobj_tbl_file                                           _sobj;
    mutable std::mutex                                          _mutex;
    mutable std::unordered_map<uint16_t, std::unique_ptr<grid>> _grids;
    std::unordered_map<uint16_t, std::vector<warp>>             _warps;
    std::unordered_map<uint16_t, std::unordered_set<uint32_t>>  _warp_tiles;
    std::unordered_map<uint16_t, std::vector<spawn>>            _spawns;
    std::unordered_map<std::string, std::vector<uint16_t>>      _habitats;
    std::unordered_map<uint16_t, size_t>                        _ranks;
    std::unordered_map<std::string, std::vector<dweller>>       _dwellers;

private:
    atlas(const std::string& maps_path, const fb::sobj_tbl_file& sobj);

public:
    static const atlas& get();
    static uint64_t     gate(uint16_t map, const position& at);
    static position     forward(const position& from, DIRECTION direction);
    static position     landing(uint16_t map);

public:
    const grid*                                             find_grid(uint16_t map) const;
    const std::vector<warp>&                                warps(uint16_t map) const;
    bool                                                    warp_at(uint16_t map, const position& at) const;
    const std::vector<spawn>&                               spawns(uint16_t map) const;
    const std::unordered_map<uint16_t, std::vector<spawn>>& spawns() const;
    std::optional<uint32_t>                                 mob(uint16_t map, uint16_t look, uint8_t color) const;
    std::vector<uint16_t> habitats(const std::unordered_set<std::string>& names) const;
    std::vector<dweller>  dwellers(const std::unordered_set<std::string>& names) const;
    bool                  reaches(uint16_t map, const position& from, const position& gate) const;
    std::optional<plan>   route(uint16_t                            from,
                                const position&                     at,
                                const std::unordered_set<uint16_t>& targets,
                                uint8_t                             level,
                                const std::unordered_set<uint64_t>& closed,
                                const std::unordered_set<uint32_t>& perilous,
                                const std::unordered_set<uint16_t>& shunned) const;
    std::optional<leap>   shortcut(uint16_t                            from,
                                   const position&                     at,
                                   const std::unordered_set<uint16_t>& targets,
                                   const std::optional<position>&      goal,
                                   uint8_t                             level,
                                   const std::unordered_set<uint64_t>& closed,
                                   const std::unordered_set<uint32_t>& perilous,
                                   const std::unordered_set<uint16_t>& shunned,
                                   const std::vector<teleport>&        teleports) const;
    std::optional<std::vector<DIRECTION>>
    path(uint16_t map, const position& from, const position& to, uint16_t range, const blocked_func& blocked) const;
};

} // namespace fb::bot

#endif
