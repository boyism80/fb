local combat   = require("brain.data.combat")
local growth   = require("brain.data.growth")
local hunting  = require("brain.data.hunting")
local magic    = require("brain.data.spells")
local fight    = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local recovery = require("brain.policies.recovery")
local score    = require("brain.policies.score")

local M = {}

local cache = {}

local function bound(snap)
    local low, high = 1, #hunting.bundles
    while low <= high do
        local middle = (low + high) // 2
        if fight.mitigate(hunting.bundles[middle].rank, snap.self.phydef or 0, 0) < snap.self.max_hp then
            low = middle + 1
        else
            high = middle - 1
        end
    end
    return high
end

local function dose(snap, kind)
    local doses = recovery.doses(snap, kind)
    return #doses > 0 and doses[#doses].amount or 0
end

local function rest(snap, result, ability, buffs)
    local charge = buffs.charge
    local time   = 0
    local taken  = result.taken
    local cast   = 1000 / magic.cast_rate
    local drink  = dose(snap, "mp")
    local food   = dose(snap, "hp")
    if result.drained > 0 and charge ~= nil and charge.chance > 0 then
        time  = time + cast / charge.chance * result.drained / math.max(snap.self.max_mp, 1)
        taken = taken + charge.cost * result.drained / math.max(snap.self.max_mp, 1)
    elseif result.drained > 0 and buffs.surge ~= nil then
        time = time + cast + result.drained / (buffs.surge.rate * math.max(snap.self.max_mp, 1)) * 1000
    elseif result.drained > 0 and drink > 0 then
        time = time + cast * result.drained / drink
    end
    if taken > 0 and ability.refill.hp > 0 then
        time = time + taken / ability.refill.hp * 1000
    elseif taken > 0 and food > 0 then
        time = time + cast * taken / food
    end
    return time
end

function M.evaluate(snap, query, floor, ability)
    local buffs   = fight.buffs(snap)
    local exp     = 0
    local time    = 0
    local kills   = 0
    local need    = 0
    local margin  = math.huge
    local deadly  = false
    local targets = {}
    local drained = 0
    local taken   = 0
    local loot    = 0
    local yields  = {}
    for _, entry in ipairs(floor.mobs) do
        local mob = hunting.mobs[entry.id]
        if mob ~= nil then
            deadly = deadly or danger.deadly(snap, query, mob)
            local result = fight.fight(snap, query, mob, floor.density)
            if result.margin > 0 and (mob.exp or 0) > 0 then
                exp     = exp + entry.count * mob.exp
                time    = time + entry.count * (result.time + rest(snap, result, ability, buffs))
                kills   = kills + entry.count
                need    = math.max(need, result.taken)
                margin  = math.min(margin, result.margin)
                drained = drained + entry.count * result.drained
                taken   = taken + entry.count * result.taken
                loot    = loot + entry.count * (mob.loot or 0)
                for _, drop in ipairs(mob.drops or {}) do
                    local found = yields[drop.name] or { rate = 0, capacity = drop.capacity }
                    found.rate = found.rate + entry.count * drop.count * drop.chance
                    yields[drop.name] = found
                end
                table.insert(targets, mob.name)
            end
        end
    end
    for _, entry in pairs(yields) do
        entry.rate = time > 0 and entry.rate / time * 1000 or 0
    end
    return {
        map     = floor.map,
        safe    = deadly == false and #targets > 0,
        rate    = time > 0 and exp / time * 1000 or 0,
        kills   = time > 0 and kills / time * 1000 or 0,
        supply  = floor.supply or 0,
        need    = need,
        margin  = margin,
        targets = targets,
        drain   = time > 0 and drained / time * 1000 or 0,
        wear    = time > 0 and taken / time * 1000 or 0,
        loot    = time > 0 and loot / time * 1000 or 0,
        yields  = yields,
    }
end

function M.horizon(snap, rate)
    local levels = growth[snap.self.class]
    local next   = levels ~= nil and levels[snap.self.level + 1] or nil
    if next == nil or rate <= 0 then
        return math.huge
    end
    return math.max(0, next.exp - (snap.self.exp or 0)) / rate
end

local function sweep(snap, query, bundle, ability)
    local floors = {}
    for _, floor in ipairs(bundle.floors) do
        local result = M.evaluate(snap, query, floor, ability)
        if result.safe == false or result.rate <= 0 then
            break
        end
        table.insert(floors, result)
    end
    return floors
end

local function candidates(snap, query, mode)
    local key = score.signature(snap, mode)
    if cache[key] == nil then
        local rested  = score.rested(snap, mode)
        local ability = score.capability(rested, query)
        local found   = {}
        for index = bound(rested), 1, -1 do
            local floors = sweep(rested, query, hunting.bundles[index], ability)
            if #floors > 0 then
                table.insert(found, { bundle = hunting.bundles[index], floors = floors })
            end
            if #found >= combat.candidates then
                break
            end
        end
        cache[key] = found
    end
    return cache[key]
end

local function share(floor, query)
    if floor.kills <= 0 then
        return 0
    end
    local others    = query.demand(floor.map)
    local effective = math.min(floor.kills, floor.supply * floor.kills / (floor.kills + others))
    return floor.rate * effective / floor.kills
end

function M.ranked(snap, query, mode)
    local ranked = {}
    for _, candidate in ipairs(candidates(snap, query, mode)) do
        local finish = 1
        local rate   = 0
        for index, floor in ipairs(candidate.floors) do
            local value = share(floor, query)
            if value > rate then
                rate   = value
                finish = index
            end
        end
        if rate > 0 then
            table.insert(ranked, { candidate = candidate, finish = finish, rate = rate })
        end
    end
    table.sort(ranked, function(left, right)
        return left.rate > right.rate
    end)
    return ranked
end

function M.reachable(snap, query, candidate)
    candidate.reachable = candidate.reachable or {}
    local shunned       = danger.shunned(snap, query)
    local key           = snap.world.map
    for _, map in ipairs(shunned) do
        key = key .. "|" .. tostring(map)
    end
    if candidate.reachable[key] == nil then
        candidate.reachable[key] = query.route({ candidate.floors[1].map }, danger.perilous(snap, query), shunned) ~= nil
    end
    return candidate.reachable[key]
end

function M.choose(snap, query, mode)
    local shunned = {}
    for _, map in ipairs(danger.shunned(snap, query)) do
        shunned[map] = true
    end
    for _, entry in ipairs(M.ranked(snap, query, mode)) do
        local candidate = entry.candidate
        local floors    = {}
        for index = 1, entry.finish do
            if shunned[candidate.floors[index].map] then
                break
            end
            table.insert(floors, candidate.floors[index])
        end
        if #floors > 0 and M.reachable(snap, query, candidate) then
            return {
                name   = candidate.bundle.name,
                floors = floors,
                rate   = entry.rate,
                need   = floors[#floors].need,
                margin = floors[#floors].margin,
                drain  = floors[#floors].drain,
                wear   = floors[#floors].wear,
                loot   = floors[#floors].loot,
                yields = floors[#floors].yields,
            }
        end
    end
    return nil
end

local names = nil

function M.need(snap, query)
    local order = snap.record.memory.order
    if order ~= nil and order.task == "HUNT" then
        if names == nil then
            names = {}
            for _, mob in pairs(hunting.mobs) do
                names[mob.name] = mob
            end
        end
        local rested = score.rested(snap)
        local need   = 0
        for _, name in ipairs(order.names or {}) do
            local mob = names[name]
            if mob ~= nil and fight.hits(mob) then
                local worst = fight.mitigate(mob.damage, snap.self.phydef or 0, 0)
                need        = math.max(need, fight.fight(rested, query, mob).taken, worst)
            end
        end
        return need
    elseif (snap.persona or {}).hunt == 1 then
        local best = M.choose(snap, query)
        return best ~= nil and best.need or 0
    else
        return 0
    end
end

function M.ready(snap, query)
    local best     = M.choose(snap, query)
    local deficits = {}
    if best == nil then
        deficits.power = true
    elseif snap.self.hp < best.need then
        deficits.hp = best.need
    end
    return { ready = next(deficits) == nil, best = best, deficits = deficits }
end

return M
