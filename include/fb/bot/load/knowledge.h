#ifndef __BOT_LOAD_KNOWLEDGE_H__
#define __BOT_LOAD_KNOWLEDGE_H__

#include <fb/model/model.h>
#include <fb/sobj_tbl_file.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fb::bot::load {

using namespace fb::model;
using namespace fb::model::enum_value;

class knowledge
{
public:
    struct grid
    {
        uint16_t              width  = 0;
        uint16_t              height = 0;
        std::vector<bool>     blocked;
        std::vector<uint8_t>  collisions;
        std::vector<uint32_t> regions;

        bool     movable(const point<uint16_t>& position) const;
        bool     passable(const point<uint16_t>& from, DIRECTION direction) const;
        uint32_t region(const point<uint16_t>& position) const;
    };

    static constexpr uint16_t NO_WORLD        = 0xFFFF;
    static constexpr uint16_t MOB_LOOK_OFFSET = 0x7FFF;

    struct warp
    {
        uint16_t        map;
        point<uint16_t> before;
        uint16_t        dest;
        point<uint16_t> after;
        uint8_t         min_level  = 0;
        uint8_t         max_level  = 0xFF;
        uint16_t        world      = NO_WORLD;
        uint16_t        world_from = 0;
        uint16_t        world_to   = 0;
    };

    struct visit
    {
        uint32_t        cost;
        point<uint16_t> at;
        const warp*     via;
    };

    struct prey
    {
        uint16_t                                                 look;
        uint32_t                                                 count;
        uint32_t                                                 exp;
        uint32_t                                                 damage;
        uint32_t                                                 hp;
        std::vector<std::pair<point<uint16_t>, point<uint16_t>>> areas;
        std::unordered_map<std::string, double>                  drops;
        double                                                   interval   = 1.0;
        bool                                                     aggressive = false;
        std::string                                              name;
    };

    struct hunting_ground
    {
        uint16_t          map;
        std::vector<prey> preys;
        point<uint16_t>   center;
    };

    struct shop
    {
        std::unordered_map<std::string, uint64_t> buys;
        std::unordered_map<std::string, uint64_t> sells;
    };

    struct npc_spot
    {
        uint16_t        map;
        uint32_t        npc;
        point<uint16_t> position;
    };

    using blocked_func = std::function<bool(const point<uint16_t>&)>;

private:
    std::unordered_map<uint16_t, grid>                         _grids;
    std::unordered_map<uint16_t, std::vector<warp>>            _warps;
    std::unordered_map<uint16_t, std::vector<npc_spot>>        _npcs;
    std::vector<hunting_ground>                                _hunting_grounds;
    std::vector<npc_spot>                                      _revive_spots;
    std::unordered_map<std::string, CLASS>                     _classes;
    std::unordered_map<uint32_t, shop>                         _shops;
    std::unordered_map<std::string, uint64_t>                  _buyable;
    std::unordered_set<std::string>                            _droppable;
    std::unordered_map<uint16_t, std::unordered_set<uint32_t>> _warp_tiles;

public:
    knowledge(const std::string&                  maps_path,
              const fb::sobj_tbl_file&            sobj,
              const std::unordered_set<uint32_t>& revive_npcs);

public:
    // clang-format off
    const grid*                                  find_grid(uint16_t map) const;
    bool                                         npc_at(uint16_t map, const point<uint16_t>& position) const;
    bool                                         warp_at(uint16_t map, const point<uint16_t>& position) const;
    bool                                         reaches(uint16_t map, const point<uint16_t>& from, const point<uint16_t>& gate) const;
    std::optional<npc_spot>                      find_npc(uint16_t map, uint32_t npc) const;
    std::unordered_map<uint16_t, visit>          search(uint16_t from, const point<uint16_t>& position, uint8_t level) const;
    std::vector<npc_spot>                        npc_spots(uint32_t npc) const;
    std::optional<CLASS>                         find_class(const std::string& name) const;
    const shop*                                  find_shop(uint32_t npc) const;
    const std::unordered_map<uint32_t, shop>&    shops() const;
    bool                                         buyable(const std::string& item) const;
    bool                                         droppable(const std::string& item) const;
    uint64_t                                     price(const std::string& item) const;
    std::vector<npc_spot>                        buyers(const std::vector<std::string>& items) const;
    std::vector<npc_spot>                        sellers(const std::string& item) const;
    const std::vector<hunting_ground>&           hunting_grounds() const;
    const std::vector<npc_spot>&                 revive_spots() const;
    std::optional<std::vector<warp>>             route(uint16_t from, const point<uint16_t>& position, uint16_t to, uint8_t level) const;
    std::unordered_map<uint16_t, uint32_t>       distances(uint16_t from, const point<uint16_t>& position, uint8_t level) const;
    std::optional<std::vector<DIRECTION>>        path(uint16_t map, const point<uint16_t>& from, const point<uint16_t>& to, uint16_t range, const blocked_func& blocked) const;
    // clang-format on
};

} // namespace fb::bot::load

#endif
