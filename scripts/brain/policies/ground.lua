local combat   = require("brain.data.combat")
local growth   = require("brain.data.growth")
local hunting  = require("brain.data.hunting")
local fight    = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local lessons  = require("brain.policies.lessons")
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
    return math.max(#doses > 0 and doses[#doses].amount or 0, recovery.purchasable(snap, kind))
end

local function paces(snap, ability, buffs)
    local cast    = fight.cast(snap)
    local max_mp  = math.max(snap.self.max_mp or 0, 1)
    local natural = combat.natural.mp * max_mp
    local mana    = natural > 0 and 1000 / natural or math.huge
    local charged = 0
    local drink   = dose(snap, "mp")
    local spells  = recovery.spells(snap)
    if buffs.charge ~= nil and buffs.charge.chance > 0 and spells.charge ~= nil then
        mana    = math.min(mana, cast / buffs.charge.chance / max_mp)
        charged = buffs.charge.cost / max_mp
    end
    if buffs.surge ~= nil then
        mana = math.min(mana, 1000 / (buffs.surge.rate * max_mp))
    end
    if drink > 0 then
        mana = math.min(mana, cast / drink)
    end
    local life = 1000 / math.max(ability.refill.hp + combat.natural.hp * math.max(snap.self.max_hp, 1), 1e-9)
    local food = dose(snap, "hp")
    if food > 0 then
        life = math.min(life, cast / food)
    end
    local heal = recovery.heal(snap)
    if heal ~= nil and heal.hp > 0 then
        life = math.min(life, (cast + heal.mp * mana) / heal.hp)
    end
    return { mana = mana, life = life, charged = charged }
end

local function rest(result, pace)
    return result.drained * pace.mana + (result.taken + result.drained * pace.charged) * pace.life
end

function M.burst(snap, mob, crowd)
    local others = math.min(combat.adjacent - 1, crowd ~= nil and crowd.extra or 0)
    return fight.spike(snap, { { mob = mob, count = 1 + others } })
end

local tables = setmetatable({}, { __mode = "k" })

local function ledger(snap, ability, crew)
    local key    = crew ~= nil and crew.key or ""
    local byself = tables[snap.self]
    if byself == nil then
        byself = {}
        tables[snap.self] = byself
    end
    local found = byself[key]
    if found == nil then
        found = { contexts = {}, cores = {}, deadly = {}, spikes = {} }
        byself[key] = found
    end
    return found
end

local function shared(snap, extra)
    local full = snap.self.mp or 0
    return setmetatable({ self = setmetatable({ mp = full / (1 + extra) }, { __index = snap.self }) }, { __index = snap })
end

local function context_of(book, snap, query, mob, crew, ability)
    local found = book.contexts[mob.id]
    if found == nil then
        found = escort.context(snap, query, mob, crew, nil, ability.refill.hp)
        book.contexts[mob.id] = found
    end
    return found
end

local function core_of(book, snap, query, mob, crew, ability, bucket)
    local bymob = book.cores[mob.id]
    if bymob == nil then
        bymob = {}
        book.cores[mob.id] = bymob
    end
    local found = bymob[bucket]
    if found == nil then
        local base    = context_of(book, snap, query, mob, crew, ability)
        local extra   = bucket * combat.bucket
        local context = setmetatable({ crowd = bucket > 0 and { extra = extra, pressure = 0 } or nil },
            { __index = base })
        local chain   = shared(snap, extra)
        found         = { chain = chain, core = survival.core(chain, query, mob, context) }
        bymob[bucket] = found
    end
    return found
end

local function assess(snap, query, floor, ability, crew)
    if crew ~= nil and #(crew.wards or {}) > 0 then
        snap = setmetatable({ aided = crew.wards, self = snap.self }, { __index = snap })
    end
    local book    = ledger(snap, ability, crew)
    local pace    = paces(snap, ability, fight.buffs(snap))
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
    local pain    = 0
    local peak    = 0
    local loot    = 0
    local stock   = 0
    local yields  = {}
    local entries = {}
    local spent   = 0
    local counted = 0
    for _, entry in ipairs(floor.mobs) do
        local mob = hunting.mobs[entry.id]
        if mob ~= nil then
            local base = core_of(book, snap, query, mob, crew, ability, 0).core
            table.insert(entries, { mob = mob, count = entry.count, result = base })
            if base.time < math.huge then
                spent   = spent + entry.count * base.time
                counted = counted + entry.count
            end
        end
    end
    local size   = crew ~= nil and crew.size or 1
    local crowd  = survival.crowd(snap, entries, counted > 0 and spent / counted or math.huge, size, floor.density)
    local split  = crew ~= nil and (1 + math.max(crew.bonus, combat.group.bonus[snap.self.class] or 0)) / size or 1
    local bucket = (crowd.extra / combat.bucket + 0.5) // 1 | 0
    local charge = fight.buffs(snap).charge
    for _, entry in ipairs(entries) do
        local mob     = entry.mob
        local slotted = core_of(book, snap, query, mob, crew, ability, bucket)
        local context = setmetatable({ crowd = crowd }, { __index = context_of(book, snap, query, mob, crew, ability) })
        local result  = survival.settle(slotted.chain, mob, context, slotted.core)
        if result.ending <= 0 then
            result = survival.fight(slotted.chain, query, mob, context)
        end
        local ending = result.ending
        local spikes = book.spikes[mob.id]
        if spikes == nil then
            spikes = {}
            book.spikes[mob.id] = spikes
        end
        local burst = spikes[bucket]
        if burst == nil then
            burst = M.burst(snap, mob, crowd)
            spikes[bucket] = burst
        end
        local deadly = book.deadly[mob.id]
        if deadly == nil then
            deadly = danger.deadly(snap, query, mob, false)
            book.deadly[mob.id] = deadly
        end
        if mob.attack == "AGGRESSIVE" and fight.hits(mob) and (ending <= burst or deadly) then
            safe = false
        end
        peak        = math.max(peak, burst)
        local bears = mob.attack == "AGGRESSIVE" or fight.hits(mob) == false or
            snap.self.max_hp - (charge ~= nil and result.drained > 0 and charge.cost or 0) > burst
        if ending > 0 and (mob.exp or 0) > 0 and bears then
            local hurt = math.max(0, result.taken - result.healed)
            exp        = exp + entry.count * mob.exp * split
            stock      = stock + entry.count * mob.exp * split
            time       = time +
                entry.count * (result.time + rest({ taken = hurt, drained = result.drained }, pace))
            kills      = kills + entry.count
            deaths     = deaths + entry.count * result.p_die
            need       = math.max(need, hurt)
            margin     = math.min(margin, ending / math.max(snap.self.max_hp, 1))
            drained    = drained + entry.count * result.drained
            taken      = taken + entry.count * hurt
            pain       = pain + entry.count * result.taken
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
        pain    = time > 0 and pain / time * 1000 or 0,
        peak    = peak,
        stock   = stock,
        crowd   = crowd.extra,
        loot    = time > 0 and loot / time * 1000 or 0,
        yields  = yields,
    }
end

local rosters = setmetatable({}, { __mode = "k" })
local assessed = {}

local function roster(floor)
    if rosters[floor] == nil then
        local parts = {}
        for _, entry in ipairs(floor.mobs) do
            table.insert(parts, entry.id .. "x" .. entry.count)
        end
        table.sort(parts)
        rosters[floor] = table.concat(parts, ",") .. "|" .. (floor.density or 0) .. "|" .. (floor.supply or 0)
    end
    return rosters[floor]
end

function M.evaluate(snap, query, floor, ability, crew)
    local owner  = (snap.self.name or "") .. (snap.hypothesis or "") .. (crew ~= nil and crew.key or "")
    local key    = table.concat({ roster(floor), snap.self.hp, snap.self.mp or 0 }, "|")
    local shared = memo.get(assessed, owner, score.signature(snap), key, function()
        return assess(snap, query, floor, ability, crew)
    end)
    return setmetatable({ map = floor.map }, { __index = shared })
end

function M.lifetime(snap, rate)
    local levels = growth[snap.self.class]
    local last   = levels ~= nil and levels[#levels] or nil
    if last == nil or rate <= 0 then
        return math.huge
    end
    return math.max(0, last.exp - (snap.self.exp or 0)) / rate
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
local budget  = 1

function M.refresh()
    budget = 1
end

function M.forget()
    for line in pairs(M.latest) do
        M.latest[line] = nil
    end
end

function M.affordable()
    if budget <= 0 then
        return false
    end
    budget = budget - 1
    return true
end

local function slot(snap, crew)
    return (snap.self.class or "") ..
        (snap.self.level or 0) .. (snap.hypothesis or "") .. (crew ~= nil and crew.key or "")
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

local positions = nil

local function position_of(name)
    if positions == nil then
        positions = {}
        for index, bundle in ipairs(hunting.bundles) do
            positions[bundle.name] = index
        end
    end
    return positions[name]
end

local candidates = nil

local function nearby(snap, query, mode, crew)
    local listed = {}
    for _, entry in ipairs(candidates(snap, query, mode, crew)) do
        local rate = 0
        for _, floor in ipairs(entry.floors) do
            rate = math.max(rate, floor.rate)
        end
        table.insert(listed, { index = position_of(entry.bundle.name), rate = rate })
    end
    table.sort(listed, function(left, right)
        return left.rate > right.rate
    end)
    local wanted = {}
    for order = 1, math.min(combat.window, #listed) do
        local index = listed[order].index
        for offset = 0, combat.window do
            wanted[index + offset] = true
        end
    end
    return wanted
end

candidates = function(snap, query, mode, crew)
    local known, context = reach(snap, query, crew)
    local owner          = slot(snap, crew)
    local base           = score.signature(snap, "")
    local signature      = score.signature(snap, mode)
    return memo.get(cache, owner, base, signature .. "|" .. context, function()
        local rested  = score.rested(snap, mode)
        local ability = score.capability(rested, query)
        local found   = {}
        local wanted  = snap.actual ~= nil and nearby(snap.actual, query, mode, crew) or nil
        for index = bound(rested), 1, -1 do
            local bundle = hunting.bundles[index]
            if known[bundle.floors[1].map] and (wanted == nil or wanted[index]) then
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
    local supply    = floor.supply * combat.rezen
    local effective = math.min(floor.kills, supply * floor.kills / (floor.kills + others))
    return floor.rate * effective / floor.kills
end

local function bonus(floor, worth)
    local total = 0
    for name, entry in pairs(floor.yields or {}) do
        total = total + entry.rate * (worth[name] or 0)
    end
    return total
end

local hops = {}

local function hop(snap, query, floors)
    if #floors < 2 then
        return 0
    end
    local key = floors[1].map .. ">" .. floors[2].map
    if hops[key] == nil then
        local spots = query.spawns(floors[1].map, floors[1].targets)
        local spot  = spots ~= nil and spots[1] or nil
        local route = spot ~= nil and query.path({ maps = { floors[2].map } }, {}, {},
            { map = floors[1].map, x = spot.x, y = spot.y, level = snap.self.level }) or nil
        hops[key]   = route ~= nil and route.cost * (snap.pace.step or combat.walk) / 1000 or math.huge
    end
    return hops[key]
end

local function lapped(snap, query, candidate, worth, moving)
    local finish = 1
    local rate   = 0
    local best   = 0
    local supply = 0
    local raw    = 0
    local stock  = 0
    local gap    = moving and hop(snap, query, candidate.floors) or 0
    for index, floor in ipairs(candidate.floors) do
        supply    = supply + share(floor, query)
        raw       = math.max(raw, floor.rate)
        stock     = stock + (floor.stock or 0)
        local lap = math.min(raw, supply)
        if index > 1 and gap > 0 then
            lap = gap < math.huge and raw > 0 and math.min(lap, stock / (stock / raw + index * gap)) or 0
        end
        local sum = lap + bonus(floor, worth)
        if sum > best then
            best   = sum
            rate   = lap
            finish = index
        end
    end
    return finish, rate, best
end

local function rank(snap, query, mode, crew, worth)
    local ranked = {}
    for _, candidate in ipairs(candidates(snap, query, mode, crew)) do
        local finish, rate, best = lapped(snap, query, candidate, worth, false)
        if best > 0 then
            table.insert(ranked, { candidate = candidate, finish = finish, rate = rate, score = best })
        end
    end
    table.sort(ranked, function(left, right)
        if left.score ~= right.score then
            return left.score > right.score
        end
        return left.candidate.bundle.name < right.candidate.bundle.name
    end)
    local settled = 0
    for _, entry in ipairs(ranked) do
        if entry.score < settled then
            break
        end
        if entry.finish > 1 then
            entry.finish, entry.rate, entry.score = lapped(snap, query, entry.candidate, worth, true)
        end
        settled = math.max(settled, entry.score)
    end
    table.sort(ranked, function(left, right)
        if left.score ~= right.score then
            return left.score > right.score
        end
        return left.candidate.bundle.name < right.candidate.bundle.name
    end)
    local nearest = nil
    local closest = math.huge
    for index, entry in ipairs(ranked) do
        if entry.score ~= ranked[1].score then
            break
        end
        local map = entry.candidate.floors[1].map
        local key = snap.world.map .. ">" .. map
        for _, floor in ipairs(entry.candidate.floors) do
            if floor.map == snap.world.map then
                hops[key] = 0
            end
        end
        if hops[key] == nil then
            local route = map == snap.world.map and { cost = 0 } or query.path({ maps = { map } }, {}, {},
                { map = snap.world.map, x = snap.world.x or 0, y = snap.world.y or 0, level = snap.self.level })
            hops[key]   = route ~= nil and route.cost or math.huge
        end
        local aim = ((snap.actual or snap).record.memory or {}).aim
        if aim == entry.candidate.bundle.name then
            nearest = index
            closest = -1
        elseif hops[key] < closest then
            nearest = index
            closest = hops[key]
        end
    end
    if nearest ~= nil and nearest > 1 then
        ranked[1], ranked[nearest] = ranked[nearest], ranked[1]
    end
    return ranked
end

local worths = {}

local function materials(snap, query, mode, crew, base)
    if snap.hypothesis ~= nil then
        return {}
    end
    local wanted = {}
    for _, open in ipairs(lessons.open(snap)) do
        for name in pairs(open.entry.materials or {}) do
            wanted[name] = 0
        end
    end
    for _, item in ipairs(snap.bag.items) do
        if wanted[item.name] ~= nil then
            wanted[item.name] = wanted[item.name] + item.count
        end
    end
    local held = {}
    for name, count in pairs(wanted) do
        held[#held + 1] = name .. "=" .. count
    end
    table.sort(held)
    local key = score.signature(snap, mode) .. "|" .. table.concat(held, ",")
    return memo.get(worths, slot(snap, crew) .. "|" .. (mode or ""), key, "", function()
        local found = {}
        for _, open in ipairs(lessons.open(snap)) do
            local missing = lessons.missing(snap, open.entry)
            if missing.drops > 0 then
                local after = M.choose(lessons.taught(snap, open.name, open.entry), query, mode, crew)
                local gain  = math.max(0, (after ~= nil and after.rate or 0) - base)
                if gain > 0 then
                    local each = gain * M.lifetime(snap, gain + base) / missing.drops
                    for name in pairs(missing.dropped) do
                        found[name] = math.max(found[name] or 0, each)
                    end
                end
            end
        end
        return found
    end)
end

local standings = {}
local latest    = {}
M.latest        = latest

function M.ranked(snap, query, mode, crew)
    local _, context = reach(snap, query, crew)
    local period     = (snap.now or 0) // combat.rerank
    local key        = table.concat({ score.signature(snap, ""), score.signature(snap, mode), context }, "|")
    local owner      = slot(snap, crew)
    local known      = memo.peek(standings, owner, period, key)
    if known ~= nil then
        return known
    end
    local line = table.concat(
        { tostring(((snap.actual or snap).record.memory or {}).lane), snap.self.name or "", snap.hypothesis or "", mode or
        "",
            tostring(crew ~= nil) }, "|")
    local last = latest[line]
    if last ~= nil and M.affordable() == false then
        return last
    end
    local found = memo.get(standings, owner, period, key, function()
        local ranked = rank(snap, query, mode, crew, {})
        local worth  = materials(snap, query, mode, crew, ranked[1] ~= nil and ranked[1].rate or 0)
        if next(worth) == nil then
            return ranked
        end
        return rank(snap, query, mode, crew, worth)
    end)
    latest[line] = found
    return found
end

function M.reachable(snap, query, candidate, crew)
    return reach(snap, query, crew)[candidate.floors[1].map] == true
end

local function peaks(floors)
    local worst = 0
    for _, floor in ipairs(floors) do
        worst = math.max(worst, floor.peak or 0)
    end
    return worst
end

local raws = nil

local function raw(map)
    if raws == nil then
        raws = {}
        for _, bundle in ipairs(hunting.bundles) do
            for _, floor in ipairs(bundle.floors) do
                raws[floor.map] = floor
            end
        end
    end
    return raws[map]
end

local function guarded(floors, query, guards)
    local kept = floors
    for _, guard in ipairs(guards or {}) do
        local limit = 0
        for index, floor in ipairs(kept) do
            local source = raw(floor.map)
            local result = source ~= nil and M.evaluate(guard.rested, query, source, guard.ability, guard.crew) or nil
            if result == nil or result.safe == false or result.rate <= 0 then
                break
            end
            limit = index
        end
        if limit < #kept then
            local trimmed = {}
            for index = 1, limit do
                table.insert(trimmed, kept[index])
            end
            kept = trimmed
        end
    end
    return kept
end

local function pick(snap, query, mode, crew, guards)
    local shunned = {}
    for _, map in ipairs(danger.shunned(snap, query)) do
        shunned[map] = true
    end
    for _, guard in ipairs(guards or {}) do
        for _, map in ipairs(danger.shunned(guard.rested, query)) do
            shunned[map] = true
        end
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
        floors = guarded(floors, query, guards)
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
                pain   = floors[#floors].pain,
                peak   = peaks(floors),
                loot   = floors[#floors].loot,
                yields = floors[#floors].yields,
                deaths = floors[#floors].deaths,
            }
        end
    end
    return nil
end

local chosen = setmetatable({}, { __mode = "k" })

function M.choose(snap, query, mode, crew, guards)
    local cached = chosen[snap]
    if cached == nil then
        cached       = {}
        chosen[snap] = cached
    end
    local key = (mode or "") .. (crew ~= nil and crew.key or "") .. (guards ~= nil and "|guarded" or "")
    if cached[key] == nil then
        local found = nil
        if mode == nil and recovery.rates(snap).mp <= 0 and #recovery.doses(snap, "mp") == 0 then
            local full = pick(snap, query, "REFILL", crew, guards)
            local dry  = pick(snap, query, "DRY", crew, guards)
            found      = (full ~= nil and (dry == nil or full.rate > dry.rate)) and full or dry
        else
            found = pick(snap, query, mode, crew, guards)
        end
        cached[key] = found or false
    end
    return cached[key] or nil
end

function M.crew(members, query)
    return escort.crew(members, query)
end

function M.mode(snap)
    return nil
end

function M.party(members, query, mode)
    local best  = nil
    local front = 1
    for index, lead in ipairs(members) do
        local others = {}
        for other, member in ipairs(members) do
            if other ~= index then
                table.insert(others, member)
            end
        end
        local guards = {}
        for _, member in ipairs(others) do
            local rest = {}
            for _, other in ipairs(members) do
                if other ~= member then
                    table.insert(rest, other)
                end
            end
            local rested = score.rested(member, M.mode(member))
            local crew = {}
            for _, other in ipairs(rest) do
                table.insert(crew, score.rested(other, M.mode(other)))
            end
            table.insert(guards,
                { rested = rested, ability = score.capability(rested, query), crew = M.crew(crew, query) })
        end
        local rested = {}
        for _, member in ipairs(others) do
            table.insert(rested, score.rested(member, M.mode(member)))
        end
        local plan = M.choose(lead, query, mode == "SUPPLY" and M.mode(lead) or mode,
            #rested > 0 and M.crew(rested, query) or nil, #guards > 0 and guards or nil)
        if plan ~= nil and (best == nil or plan.rate > best.rate) then
            best  = plan
            front = index
        end
    end
    return best, front
end

local function held(members, index, query, bundle)
    local lead    = score.rested(members[index], M.mode(members[index]))
    local ability = score.capability(lead, query)
    local others  = {}
    for other, member in ipairs(members) do
        if other ~= index then
            table.insert(others, score.rested(member, M.mode(member)))
        end
    end
    local guards = {}
    for slot, member in ipairs(others) do
        local rest = { lead }
        for other, entry in ipairs(others) do
            if other ~= slot then
                table.insert(rest, entry)
            end
        end
        table.insert(guards, { rested = member, ability = score.capability(member, query), crew = M.crew(rest, query) })
    end
    local shunned = {}
    for _, member in ipairs(members) do
        for _, map in ipairs(danger.shunned(member, query)) do
            shunned[map] = true
        end
    end
    local floors = {}
    for _, floor in ipairs(sweep(lead, query, bundle, ability, #others > 0 and M.crew(others, query) or nil)) do
        if shunned[floor.map] then
            break
        end
        table.insert(floors, floor)
    end
    return guarded(floors, query, guards)
end

function M.hold(members, query, name)
    local bundle = nil
    for _, entry in ipairs(hunting.bundles) do
        if entry.name == name then
            bundle = entry
        end
    end
    if bundle == nil then
        return nil
    end
    local floors = {}
    for index = 1, #members do
        local found = held(members, index, query, bundle)
        if #found > #floors then
            floors = found
        end
    end
    if #floors == 0 then
        return nil
    end
    local rate = 0
    for _, floor in ipairs(floors) do
        rate = math.max(rate, floor.rate)
    end
    local last = floors[#floors]
    return {
        name   = bundle.name,
        floors = floors,
        rate   = rate,
        need   = last.need,
        margin = last.margin,
        drain  = last.drain,
        spend  = last.spend,
        wear   = last.wear,
        pain   = last.pain,
        peak   = peaks(floors),
        loot   = last.loot,
        yields = last.yields,
        deaths = last.deaths,
    }
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
