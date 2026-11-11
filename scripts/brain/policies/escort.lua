local combat   = require("brain.data.combat")
local magic    = require("brain.data.spells")
local fight    = require("brain.policies.combat")
local score    = require("brain.policies.score")
local survival = require("brain.policies.survival")

local M = {}

local presents = setmetatable({}, { __mode = "k" })

function M.member(profile)
    return {
        self      = profile.self,
        spells    = profile.spells or {},
        gear      = profile.gear or {},
        buffs     = profile.buffs or {},
        bag       = profile.bag or { items = {}, used = 0, capacity = 0 },
        pace      = profile.pace or {},
        world     = { map = profile.world.map, x = profile.world.x, y = profile.world.y, mobs = {}, items = {} },
        record    = { failures = {}, memory = { fled = profile.fled, dry = profile.dry } },
        now       = profile.now,
        plan      = profile.plan,
        rally     = profile.rally,
        vanguard  = profile.vanguard,
        battle    = profile.battle,
        heading   = profile.heading,
        pinned    = profile.pinned,
        walled    = profile.walled,
        peril     = profile.peril,
        dealt     = profile.dealt,
        rescue    = profile.rescue,
        going     = profile.going,
        errand    = profile.errand,
        servicing = profile.servicing,
        arrives   = profile.arrives,
        task      = profile.task,
        fleeing   = profile.fleeing,
        fallen    = profile.fallen,
        joined    = profile.joined,
        leashed   = profile.leashed,
        looted    = profile.looted or 0,
    }
end

function M.crew(members, query)
    local keys  = {}
    local heals = {}
    local names = {}
    local heal  = 0
    local stock = 0
    local bonus = 0
    for index, member in ipairs(members) do
        table.insert(keys, score.signature(member, ""))
        table.insert(names, member.self.name)
        local hps    = score.capability(member, query).hps
        heals[index] = hps.others
        heal         = heal + heals[index]
        stock        = stock + hps.stock
        bonus        = math.max(bonus, combat.group.bonus[member.self.class] or 0)
    end
    local assist = {}
    for _, member in ipairs(members) do
        for _, learned in ipairs(member.spells or {}) do
            local data = magic.spells[learned.name]
            if data ~= nil and data.role == "DEBUFF" and type(data.mp or 0) == "number" and
                (data.mp or 0) <= (member.self.mp or 0) then
                assist[learned.name] = true
            end
        end
    end
    local wards = {}
    for _, member in ipairs(members) do
        for _, learned in ipairs(member.spells or {}) do
            local data = magic.spells[learned.name]
            if data ~= nil and data.role == "BUFF" and data.self == false and data.others == true and data.boost ~= nil then
                table.insert(wards, learned.name)
            end
        end
    end
    table.sort(wards)
    return {
        members = members,
        names   = names,
        wards   = wards,
        assist  = assist,
        size    = #members + 1,
        heals   = heals,
        heal    = heal,
        stock   = stock,
        support = heal,
        bonus   = #members > 0 and bonus or 0,
        key     = "|crew:" .. table.concat(keys, ";"),
    }
end

local fields = setmetatable({}, { __mode = "k" })

function M.reach()
    return math.min(combat.sight.x, combat.sight.y)
end

function M.steps(snap, query, member, tiles)
    local known = fields[snap]
    if known == nil then
        known        = {}
        fields[snap] = known
    end
    local world = member.world
    local own   = known[member.self.name or ""]
    if own == nil or own.map ~= world.map or own.x ~= world.x or own.y ~= world.y then
        own                           = { map = world.map, x = world.x, y = world.y, tiles = {} }
        known[member.self.name or ""] = own
    end
    local goals = {}
    local keys  = {}
    for _, tile in ipairs(tiles) do
        local key = tile.x .. "," .. tile.y
        if own.tiles[key] == nil and keys[key] == nil then
            table.insert(goals, { x = tile.x, y = tile.y, range = 0 })
            keys[key] = #goals
        end
    end
    if #goals > 0 then
        local found = query ~= nil and query.steps ~= nil and query.steps(world.map, world.x, world.y, goals) or nil
        for key, index in pairs(keys) do
            local goal     = goals[index]
            local value    = found == nil and math.abs(goal.x - world.x) + math.abs(goal.y - world.y) or found[index]
            own.tiles[key] = type(value) == "number" and value or false
        end
    end
    local result = {}
    for index, tile in ipairs(tiles) do
        result[index] = own.tiles[tile.x .. "," .. tile.y]
    end
    return result
end

-- Together means in sight and no farther to walk than the corner of sight is on open ground: a crewmate in
-- sight across a wall that takes a long walk round cannot come to a fight in time. Walking reach itself would
-- drop crewmates in plain sight on open ground; how near is better is the gap cost's to weigh (spacing).
local function near(snap, query, member)
    local world = member.world
    if member.self == nil or member.self.ghost or world.map ~= snap.world.map or world.x == nil then
        return false
    end
    local staying = member.heading == nil
    for _, map in ipairs(member.heading or {}) do
        staying = staying or map == snap.world.map
    end
    if staying == false then
        return false
    end
    if math.abs(world.x - snap.world.x) > combat.sight.x or math.abs(world.y - snap.world.y) > combat.sight.y then
        return false
    end
    local steps = M.steps(snap, query, member, { { x = snap.world.x, y = snap.world.y } })[1]
    return type(steps) == "number" and steps <= combat.sight.x + combat.sight.y
end

-- A tile is with the crew when it is in each crewmate's sight, and when I walk to it also no farther for the
-- crewmate to walk than the corner of sight is on open ground, as a crewmate is (near). Counted by sight alone,
-- skeletons across a wall five tiles from a poet were with the crew, and its warrior walked about 40 steps round
-- the wall for one while the poet that followed was caught on the way and fought alone (offline profile pair
-- rooted striker seed 1, 33-45s, run 13:15). A tile I strike from where I stand needs no walk.
function M.tethered(snap, query, x, y, walking)
    for _, profile in ipairs(snap.party or {}) do
        local world = profile.world
        if world ~= nil and profile.self ~= nil and profile.self.ghost == false and world.map == snap.world.map and
            world.x ~= nil then
            if math.abs(world.x - x) > combat.sight.x or math.abs(world.y - y) > combat.sight.y then
                return false
            end
            if walking then
                local steps = M.steps(snap, query, M.member(profile), { { x = x, y = y } })[1]
                if type(steps) ~= "number" or steps > combat.sight.x + combat.sight.y then
                    return false
                end
            end
        end
    end
    return true
end

function M.center(snap)
    local x, y, count = 0, 0, 0
    for _, profile in ipairs(snap.party or {}) do
        local world = profile.world
        if world ~= nil and profile.self ~= nil and profile.self.ghost == false and world.map == snap.world.map and
            world.x ~= nil then
            x     = x + world.x
            y     = y + world.y
            count = count + 1
        end
    end
    return count > 0 and { x = (x / count + 0.5) // 1 | 0, y = (y / count + 0.5) // 1 | 0 } or nil
end

function M.ready(crew, window)
    return crew ~= nil and math.min(crew.heal * window / 1000, crew.stock) or 0
end

function M.present(snap, query)
    if presents[snap] == nil then
        local found = {}
        for _, profile in ipairs(snap.party or {}) do
            local member = M.member(profile)
            if near(snap, query, member) then
                table.insert(found, member)
            end
        end
        presents[snap] = #found > 0 and M.crew(found, query) or false
    end
    return presents[snap] or nil
end

function M.complete(snap, query)
    local crew = M.present(snap, query)
    return #(snap.party or {}) > 0 and crew ~= nil and #crew.members == #snap.party
end

local function arriving(snap, member)
    local coming = false
    for _, map in ipairs(member.heading or {}) do
        coming = coming or map == snap.world.map
    end
    if coming == false or member.arrives == nil or member.self == nil or member.self.ghost then
        return false
    end
    return member.task == "FIELD" and
        member.arrives + 2 * combat.adjacent * (member.pace.step or combat.walk) >= snap.now
end

local gaps = {}

local function nearby(snap, query, member)
    local world = member.world
    if member.task == "ESCAPE" or member.self.ghost or world.map == nil or world.map == snap.world.map or world.x == nil then
        return false
    end
    -- The member's route is limited by its level, so the level is part of the key.
    local key = table.concat({ world.map, world.x // combat.sight.x, world.y // combat.sight.y, snap.world.map,
        member.self.level or 0 }, "|")
    if gaps[key] == nil then
        local route = query.path({ maps = { snap.world.map } }, {}, {},
            { map = world.map, x = world.x, y = world.y, level = member.self.level })
        gaps[key]   = route ~= nil and route.cost or math.huge
    end
    return gaps[key] <= 2 * combat.sight.x
end

local gathered = setmetatable({}, { __mode = "k" })

function M.gathering(snap, query)
    if gathered[snap] ~= nil then
        return gathered[snap] or nil
    end
    local found = {}
    for _, profile in ipairs(snap.party or {}) do
        local member = M.member(profile)
        if near(snap, query, member) or arriving(snap, member) or nearby(snap, query, member) then
            table.insert(found, member)
        end
    end
    gathered[snap] = #found > 0 and M.crew(found, query) or false
    return gathered[snap] or nil
end

local function arrange(snap, query, mob, crew, density, own, share)
    local support = share * crew.heal
    local aid     = (support + own) / 1000
    local speed   = 0
    for index, member in ipairs(crew.members) do
        local helper = fight.outcome(member, query, mob, density, aid)
        if helper.time < math.huge then
            local busy = crew.heals[index] > 0 and share or 0
            speed      = speed + (1 - busy) / helper.time
        end
    end
    return { density = density, aid = aid, speed = speed, support = support, share = share, assist = crew.assist }
end

local function clamp(value)
    return math.max(0, math.min(1, value))
end

function M.context(snap, query, mob, crew, density, own)
    own = own or 0
    if crew == nil then
        return { density = density, aid = own > 0 and own / 1000 or nil }
    end
    if crew.heal <= 0 then
        return arrange(snap, query, mob, crew, density, own, 0)
    end
    local shares = { 1 }
    local idle   = arrange(snap, query, mob, crew, density, own, 0)
    local alone  = survival.fight(snap, query, mob, idle)
    if alone.time < math.huge and alone.time > 0 then
        table.insert(shares, clamp((alone.taken - alone.healed) / alone.time * 1000 / crew.heal))
    end
    local opener = fight.opener(snap, mob)
    if opener ~= nil and opener.hp > 0 then
        table.insert(shares, clamp((opener.hp * 1000 / fight.cast(snap) - own) / crew.heal))
    end
    local best   = nil
    local result = nil
    for _, share in ipairs(shares) do
        local context = arrange(snap, query, mob, crew, density, own, share)
        local fought  = survival.fight(snap, query, mob, context)
        local better  = result == nil or
            (fought.ending > 0 and (result.ending <= 0 or fought.time < result.time)) or
            (fought.ending <= 0 and result.ending <= 0 and fought.ending > result.ending)
        if better then
            best   = context
            result = fought
        end
    end
    return best
end

return M
