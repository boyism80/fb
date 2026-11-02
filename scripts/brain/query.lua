local profile = require("brain.profile")

local M = {}

function M.bind(brain, bot)
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
            return brain:demand(bot, map)
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
