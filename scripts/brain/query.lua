local profile = require("brain.profile")

local M = {}

function M.bind(brain, bot)
    local demands = {}
    return {
        npcs = function(names)
            return brain:npcs(bot, names)
        end,
        item = function(name)
            return brain:item(name)
        end,
        native = function(map, mob)
            return brain:native(bot, map, mob)
        end,
        habitats = function(names)
            return brain:habitats(bot, names)
        end,
        spawns = function(map, names)
            return brain:spawns(bot, names, map)
        end,
        demand = function(map)
            local found = demands[map]
            if found == nil then
                found = brain:demand(bot, map)
                demands[map] = found
            end
            return found
        end,
        roster = function()
            return brain:roster(bot)
        end,
        steps = function(map, x, y, goals)
            return brain:steps(map, x, y, goals)
        end,
        ally = function(name)
            return brain:ally(bot, name)
        end,
        warp = function(map, x, y)
            return brain:warp_at(map, x, y)
        end,
        reach = function(perilous, shunned)
            return profile.wait(bot:name(), brain.reach, brain, bot,
                { perilous = perilous or {}, shunned = shunned or {} })
        end,
        route = function(maps, perilous, shunned)
            return profile.wait(bot:name(), brain.route, brain, bot, { maps = maps },
                { perilous = perilous or {}, shunned = shunned or {} })
        end,
        path = function(dest, perilous, shunned, origin)
            return profile.wait(bot:name(), brain.route, brain, bot, dest,
                { perilous = perilous or {}, shunned = shunned or {} },
                origin)
        end,
    }
end

return M
