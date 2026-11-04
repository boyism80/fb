local M = {}

local npcs  = {}
local items = {}

local function origin(snap, from)
    local place = from or snap.world
    return { map = place.map, x = place.x, y = place.y, level = snap.self.level }
end

function M.bind(mind, snap, world)
    world = world or {}
    return {
        npcs = function(names)
            local key = table.concat(names, "|")
            if npcs[key] == nil then
                npcs[key] = mind:npcs(nil, names)
            end
            return npcs[key]
        end,
        item = function(name)
            local found = items[name]
            if found == nil then
                found = mind:item(name) or false
                items[name] = found
            end
            return found or nil
        end,
        native = function(map, mob)
            return mind:native(nil, map, mob)
        end,
        habitats = function(names)
            return mind:habitats(nil, names)
        end,
        spawns = function(map, names)
            return mind:spawns(nil, names, map or snap.world.map)
        end,
        demand = function(map)
            return (world.demand or {})[map] or 0
        end,
        roster = function()
            return world.roster or {}
        end,
        steps = function(map, x, y, goals)
            return mind:steps(map, x, y, goals)
        end,
        ally = function(name)
            return world.ally ~= nil and world.ally(name) or nil
        end,
        reach = function(perilous, shunned)
            return mind:reach(nil, { perilous = perilous or {}, shunned = shunned or {} }, origin(snap))
        end,
        route = function(maps, perilous, shunned)
            return mind:route(nil, { maps = maps }, { perilous = perilous or {}, shunned = shunned or {} },
                origin(snap))
        end,
        path = function(dest, perilous, shunned, from)
            return mind:route(nil, dest, { perilous = perilous or {}, shunned = shunned or {} }, origin(snap, from))
        end,
    }
end

return M
