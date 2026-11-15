local terrain = require("brain.policies.terrain")
local profile = require("brain.profile")

local M = {}

local function asked(kind)
    local info = debug.getinfo(3, "Sl")
    return info ~= nil and string.format("%s@%s:%d", kind, info.short_src:match("([%w_]+)%.lua$") or "?",
        info.currentline) or kind
end

function M.bind(brain, bot, snap)
    local demands = {}
    return {
        snap = snap,
        plan = function(key, request)
            brain:plan(bot, key, request)
        end,
        planned = function(known)
            return brain:planned(bot, known)
        end,
        await_plan = function(key)
            return profile.wait(bot:name(), "plan", brain.await_plan, brain, bot, key)
        end,
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
        demand = function(map, crew)
            local key   = map .. "|" .. table.concat(crew or {}, ",")
            local found = demands[key]
            if found == nil then
                found = brain:demand(bot, map, crew or {})
                demands[key] = found
            end
            return found
        end,
        roster = function()
            return brain:roster(bot)
        end,
        steps = function(map, x, y, goals, traced, strict)
            return terrain.steps(brain, map, x, y, goals, terrain.blocking(snap, map, x, y), traced, strict)
        end,
        detour = function(map, from, to, avoided)
            return terrain.detour(brain, map, from, to, avoided)
        end,
        walled = function(map, x, y, goals, walls)
            return brain:steps(map, x, y, goals, walls)
        end,
        floor = function(map, x, y)
            return terrain.floor(brain, map, x, y)
        end,
        ally = function(name)
            return brain:ally(bot, name)
        end,
        warp = function(map, x, y)
            return brain:warp_at(map, x, y)
        end,
        lane = function(map, x, y)
            return brain:lane(map, x, y)
        end,
        untangle = function(map, movers, blocked, limit)
            return brain:untangle(bot, map, movers, blocked, limit)
        end,
        direct = function(name, plan)
            return brain:direct(name, plan)
        end,
        directed = function(name)
            return brain:directed(name)
        end,
        present = function(name, at)
            return brain:present(name, at)
        end,
        presences = function(except)
            return brain:presences(except)
        end,
        region = function(map, x, y)
            return brain:region(map, x, y)
        end,
        reach = function(perilous, shunned, from)
            return profile.wait(bot:name(), asked("reach"), brain.reach, brain, bot,
                { perilous = perilous or {}, shunned = shunned or {} }, from)
        end,
        leaps = function(destination, detour, origin)
            return profile.wait(bot:name(), "leaps", brain.leaps, brain, bot, destination, detour, origin)
        end,
        distances = function(detour, origin)
            return profile.wait(bot:name(), "distances", brain.distances, brain, bot, detour, origin)
        end,
        route = function(maps, perilous, shunned)
            return profile.wait(bot:name(), asked("route"), brain.route, brain, bot, { maps = maps },
                { perilous = perilous or {}, shunned = shunned or {} })
        end,
        path = function(dest, perilous, shunned, origin)
            return profile.wait(bot:name(), asked("path"), brain.route, brain, bot, dest,
                { perilous = perilous or {}, shunned = shunned or {} },
                origin)
        end,
    }
end

return M
