local combat   = require("brain.data.combat")
local growth   = require("brain.data.growth")
local hunting  = require("brain.data.hunting")
local fight    = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local memo     = require("brain.policies.memo")
local recovery = require("brain.policies.recovery")
local survival = require("brain.policies.survival")
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
    local cast   = fight.cast(snap)
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

function M.evaluate(snap, query, floor, ability, crew)
    local buffs   = fight.buffs(snap)
    local exp     = 0
    local time    = 0
    local kills   = 0
    local need    = 0
    local margin  = math.huge
    local safe    = true
    local deaths  = 0
    local targets = {}
    local drained = 0
    local taken   = 0
    local loot    = 0
    local yields  = {}
    local entries = {}
    local spent   = 0
    local counted = 0
    for _, entry in ipairs(floor.mobs) do
        local mob = hunting.mobs[entry.id]
        if mob ~= nil then
            local context = escort.context(snap, query, mob, crew, floor.density, ability.refill.hp)
            local result  = survival.fight(snap, query, mob, context)
            table.insert(entries, { mob = mob, count = entry.count, result = result, context = context })
            if result.time < math.huge then
                spent   = spent + entry.count * result.time
                counted = counted + entry.count
            end
        end
    end
    local size  = crew ~= nil and crew.size or 1
    local crowd = survival.crowd(snap, entries, counted > 0 and spent / counted or math.huge, size, floor.density)
    local split = crew ~= nil and (1 + math.max(crew.bonus, combat.group.bonus[snap.self.class] or 0)) / size or 1
    for _, entry in ipairs(entries) do
        local mob           = entry.mob
        entry.context.crowd = crowd
        local result        = survival.fight(snap, query, mob, entry.context)
        local ending        = result.ending
        if mob.attack == "AGGRESSIVE" and fight.hits(mob) and ending <= 0 then
            safe = false
        end
        if ending > 0 and (mob.exp or 0) > 0 then
            local hurt = math.max(0, result.taken - result.healed)
            exp        = exp + entry.count * mob.exp * split
            time       = time +
                entry.count * (result.time + rest(snap, { taken = hurt, drained = result.drained }, ability, buffs))
            kills      = kills + entry.count
            deaths     = deaths + entry.count * result.p_die
            need       = math.max(need, hurt)
            margin     = math.min(margin, ending / math.max(snap.self.max_hp, 1))
            drained    = drained + entry.count * result.drained
            taken      = taken + entry.count * hurt
            loot       = loot + entry.count * (mob.loot or 0)
            for _, drop in ipairs(mob.drops or {}) do
                local found = yields[drop.name] or { rate = 0, capacity = drop.capacity }
                found.rate = found.rate + entry.count * drop.count * drop.chance
                yields[drop.name] = found
            end
            table.insert(targets, mob.name)
        end
    end
    if time > 0 and deaths > 0 then
        exp = exp - deaths * (danger.penalty(snap) + danger.revival(snap, query) * exp / time * 1000)
    end
    for _, entry in pairs(yields) do
        entry.rate = time > 0 and entry.rate / time * 1000 or 0
    end
    return {
        map     = floor.map,
        safe    = safe and #targets > 0,
        rate    = time > 0 and exp / time * 1000 or 0,
        kills   = time > 0 and kills / time * 1000 or 0,
        deaths  = time > 0 and deaths / time * 1000 or 0,
        supply  = floor.supply or 0,
        need    = need,
        margin  = margin,
        targets = targets,
        drain   = time > 0 and drained / time * 1000 or 0,
        spend   = kills > 0 and drained / kills or 0,
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

local function sweep(snap, query, bundle, ability, crew)
    local floors = {}
    for _, floor in ipairs(bundle.floors) do
        local result = M.evaluate(snap, query, floor, ability, crew)
        if result.safe == false or result.rate <= 0 then
            break
        end
        table.insert(floors, result)
    end
    return floors
end

local reaches = {}
local sweeps  = {}

local function slot(snap, crew)
    return (snap.self.name or "") .. (snap.hypothesis or "") .. (crew ~= nil and crew.key or "")
end

local function reach(snap, query, crew)
    snap          = snap.actual or snap
    local shunned = danger.shunned(snap, query)
    local base    = table.concat({ snap.world.map, snap.self.level, score.signature(snap), table.concat(shunned, ",") },
        "|")
    return memo.get(reaches, slot(snap, crew), base, "", function()
        return query.reach(danger.perilous(snap, query, crew or false), shunned) or {}
    end), base
end

local function candidates(snap, query, mode, crew)
    local known, context = reach(snap, query, crew)
    local owner          = slot(snap, crew)
    local base           = score.signature(snap, "")
    local signature      = score.signature(snap, mode)
    return memo.get(cache, owner, base, signature .. "|" .. context, function()
        local rested  = score.rested(snap, mode)
        local ability = score.capability(rested, query)
        local found   = {}
        for index = bound(rested), 1, -1 do
            local bundle = hunting.bundles[index]
            if known[bundle.floors[1].map] then
                local floors = memo.get(sweeps, owner, base, signature .. "|" .. index, function()
                    return sweep(rested, query, bundle, ability, crew)
                end)
                if #floors > 0 then
                    table.insert(found, { bundle = bundle, floors = floors })
                end
            end
            if #found >= combat.candidates then
                break
            end
        end
        return found
    end)
end

local function share(floor, query)
    if floor.kills <= 0 then
        return 0
    end
    local others    = query.demand(floor.map)
    local effective = math.min(floor.kills, floor.supply * floor.kills / (floor.kills + others))
    return floor.rate * effective / floor.kills
end

function M.ranked(snap, query, mode, crew)
    local ranked = {}
    for _, candidate in ipairs(candidates(snap, query, mode, crew)) do
        local finish = 1
        local rate   = 0
        local supply = 0
        local raw    = 0
        for index, floor in ipairs(candidate.floors) do
            supply    = supply + share(floor, query)
            raw       = math.max(raw, floor.rate)
            local lap = math.min(raw, supply)
            if lap > rate then
                rate   = lap
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

function M.reachable(snap, query, candidate, crew)
    return reach(snap, query, crew)[candidate.floors[1].map] == true
end

local function pick(snap, query, mode, crew)
    local shunned = {}
    for _, map in ipairs(danger.shunned(snap, query)) do
        shunned[map] = true
    end
    for _, entry in ipairs(M.ranked(snap, query, mode, crew)) do
        local candidate = entry.candidate
        local floors    = {}
        for index = 1, entry.finish do
            if shunned[candidate.floors[index].map] then
                break
            end
            table.insert(floors, candidate.floors[index])
        end
        if #floors > 0 and M.reachable(snap, query, candidate, crew) then
            return {
                name   = candidate.bundle.name,
                floors = floors,
                rate   = entry.rate,
                need   = floors[#floors].need,
                margin = floors[#floors].margin,
                drain  = floors[#floors].drain,
                spend  = floors[#floors].spend,
                wear   = floors[#floors].wear,
                loot   = floors[#floors].loot,
                yields = floors[#floors].yields,
                deaths = floors[#floors].deaths,
            }
        end
    end
    return nil
end

local chosen = setmetatable({}, { __mode = "k" })

function M.choose(snap, query, mode, crew)
    local cached = chosen[snap]
    if cached == nil then
        cached       = {}
        chosen[snap] = cached
    end
    local key = (mode or "") .. (crew ~= nil and crew.key or "")
    if cached[key] == nil then
        cached[key] = pick(snap, query, mode, crew) or false
    end
    return cached[key] or nil
end

function M.crew(members, query)
    return escort.crew(members, query)
end

function M.mode(snap)
    return recovery.potable(snap) and "REFILL" or nil
end

function M.party(members, query, mode)
    local front = 1
    local ehp   = -math.huge
    for index, member in ipairs(members) do
        local ability = score.capability(member, query)
        if ability.ehp > ehp then
            front = index
            ehp   = ability.ehp
        end
    end
    local others = {}
    for index, member in ipairs(members) do
        if index ~= front then
            table.insert(others, member)
        end
    end
    local lead = members[front]
    if mode == "SUPPLY" then
        mode = M.mode(lead)
    end
    return M.choose(lead, query, mode, #others > 0 and M.crew(others, query) or nil), front
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
                need        = math.max(need, fight.outcome(rested, query, mob).taken, worst)
            end
        end
        return need
    else
        local best = M.choose(snap, query)
        return best ~= nil and best.need or 0
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
