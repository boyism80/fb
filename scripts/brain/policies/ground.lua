local combat   = require("brain.data.combat")
local growth   = require("brain.data.growth")
local hunting  = require("brain.data.hunting")
local teleport = require("brain.data.teleport")
local fight    = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local lessons  = require("brain.policies.lessons")
local memo     = require("brain.policies.memo")
local recovery = require("brain.policies.recovery")
local survival = require("brain.policies.survival")
local score    = require("brain.policies.score")

local M = {}

local NEAREST = 0.64

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

local trips = {}

local function restock(snap, query, floor)
    -- The route home passes warps with level limits, so the level is part of the key.
    -- Without it a cost cached for another bot's level (often "no route") froze low-level solo bots.
    local key = table.concat({ floor.map, snap.self.nation or 0, snap.self.level or 0 }, "|")
    if trips[key] == nil then
        local maps  = teleport.returns[snap.self.nation] or {}
        local route = #maps > 0 and query.path({ maps = maps }, {}, {},
            { map = floor.map, x = 0, y = 0, level = snap.self.level }) or nil
        trips[key]  = route ~= nil and route.cost or false
    end
    return trips[key] ~= false and 2 * trips[key] * (snap.pace.step or combat.walk) or math.huge
end

local function paces(snap, ability, buffs, trip)
    local cast    = fight.cast(snap)
    local max_mp  = math.max(snap.self.max_mp or 0, 1)
    local mana    = math.huge
    local charged = 0
    local drink   = recovery.bite(snap, "mp")
    local spells  = recovery.spells(snap)
    if buffs.charge ~= nil and buffs.charge.chance > 0 and spells.charge ~= nil then
        mana    = math.min(mana, cast / buffs.charge.chance / max_mp)
        charged = buffs.charge.cost / max_mp
    end
    if buffs.surge ~= nil then
        mana = math.min(mana, 1000 / (buffs.surge.rate * max_mp))
    end
    if drink > 0 then
        mana = math.min(mana, cast / drink + trip / math.max(recovery.carriable(snap, "mp"), drink))
    end
    local life = 1000 / math.max(ability.refill.hp, 1e-9)
    local food = recovery.bite(snap, "hp")
    if food > 0 then
        life = math.min(life, cast / food + trip / math.max(recovery.carriable(snap, "hp"), food))
    end
    local heal = recovery.heal(snap)
    if heal ~= nil and heal.hp > 0 then
        life = math.min(life, (cast + heal.mp * mana) / heal.hp)
    end
    return { mana = mana, life = life, charged = charged }
end

local function rest(result, pace)
    local mana = result.drained > 0 and result.drained * pace.mana or 0
    local life = result.taken + result.drained * pace.charged
    return mana + (life > 0 and life * pace.life or 0)
end

function M.burst(snap, mob, crowd)
    local others = math.min(combat.adjacent - 1, crowd ~= nil and crowd.extra or 0)
    return fight.spike(snap, { { mob = mob, count = 1 + others } })
end

local tables = setmetatable({}, { __mode = "k" })

local perishing = {}

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
    local pace    = paces(snap, ability, fight.buffs(snap), restock(snap, query, floor))
    local exp     = 0
    local time    = 0
    local kills   = 0
    local need    = 0
    local margin  = math.huge
    local safe    = true
    local deaths  = 0
    local targets = {}
    local drained = 0
    local crewed  = {}
    local taken   = 0
    local pain    = 0
    local fought  = 0
    local peak    = 0
    local loot    = 0
    local stock   = 0
    local yields  = {}
    local entries = {}
    local spent   = 0
    local counted = 0
    local travel  = (floor.density or 0) > 0 and NEAREST / math.sqrt(floor.density) * (snap.pace.step or combat.walk) or
        0
    for _, entry in ipairs(floor.mobs) do
        local mob = hunting.mobs[entry.id]
        if mob ~= nil then
            local base = core_of(book, snap, query, mob, crew, ability, 0).core
            table.insert(entries, { mob = mob, count = entry.count, result = base })
            if base.time < math.huge then
                local hurt    = math.max(0, base.taken - base.healed)
                local resting = (hurt > 0 or base.drained > 0) and rest({ taken = hurt, drained = base.drained }, pace) or
                    0
                spent         = spent + entry.count * (base.time + travel + (resting < math.huge and resting or 0))
                counted       = counted + entry.count
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
            -- A floor is safe for the crew that hunts it: told alone, a mob two kill together barred the floor
            -- from the pair as well, and two mages planned below the grounds they could hold together (user, 11-13).
            -- Weighed with the crew it is kept past this plan by the build, the rested mode and the crew: worked out
            -- again for every plan, it took a quarter of the decision time (offline profile, run 16:14).
            deadly = memo.get(perishing, snap.self.name, score.signature(snap, snap.mode or ""),
                mob.id .. (crew ~= nil and crew.key or ""), function()
                    return danger.deadly(snap, query, mob, crew or false)
                end)
            book.deadly[mob.id] = deadly
        end
        if fight.seeks(mob) and fight.hits(mob) and deadly then
            safe = false
        end
        local close = fight.seeks(mob) and fight.hits(mob) and burst > 0 and
            math.max(0, math.min(1, (2 * burst - ending) / burst)) or 0
        peak        = math.max(peak, burst)
        local bears = fight.seeks(mob) or fight.hits(mob) == false or
            snap.self.max_hp - (charge ~= nil and result.drained > 0 and charge.cost or 0) > burst
        if ending > 0 and (mob.exp or 0) > 0 and bears then
            local hurt = math.max(0, result.taken - result.healed)
            local each = fight.capped(snap, mob.exp * split)
            exp        = exp + entry.count * each
            stock      = stock + entry.count * each
            time       = time +
                entry.count * (result.time + travel + rest({ taken = hurt, drained = result.drained }, pace))
            kills      = kills + entry.count
            deaths     = deaths + entry.count * (1 - (1 - result.p_die) * (1 - close))
            need       = math.max(need, hurt)
            margin     = math.min(margin, ending / math.max(snap.self.max_hp, 1))
            drained    = drained + entry.count * result.drained
            for name, rate in pairs(context.drains or {}) do
                crewed[name] = (crewed[name] or 0) + entry.count * result.time * rate
            end
            taken  = taken + entry.count * hurt
            pain   = pain + entry.count * result.taken
            fought = fought + entry.count * result.time
            loot   = loot + entry.count * (mob.loot or 0)
            for _, drop in ipairs(mob.drops or {}) do
                local found = yields[drop.name] or { rate = 0, capacity = drop.capacity }
                found.rate = found.rate + entry.count * drop.count * drop.chance
                yields[drop.name] = found
            end
            table.insert(targets, mob.name)
        end
    end
    if time > 0 and deaths > 0 then
        exp = exp - deaths * (danger.penalty(snap) + danger.revival(snap, query, floor.map) * exp / time * 1000)
    end
    for _, entry in pairs(yields) do
        entry.rate = time > 0 and entry.rate / time * 1000 or 0
    end
    local drains = {}
    for _, member in ipairs(crew ~= nil and crew.members or {}) do
        local name   = member.self.name
        drains[name] = (time > 0 and (crewed[name] or 0) / time * 1000 or 0) + recovery.upkeep(member)
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
        drain   = (time > 0 and drained / time * 1000 or 0) + recovery.upkeep(snap),
        drains  = drains,
        spend   = kills > 0 and drained / kills or 0,
        wear    = time > 0 and taken / time * 1000 or 0,
        pain    = time > 0 and pain / time * 1000 or 0,
        press   = fought > 0 and pain / fought * 1000 or 0,
        peak    = peak,
        stock   = stock,
        walk    = travel,
        mobs    = counted,
        density = floor.density,
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
    -- Active buffs change the fight forecast (a cloak multiplies the opening strike), so they are part of
    -- the key; the signature only strips their stat bonuses. Without them a rogue kept a forecast made
    -- before or after its cloak and swung where its skill paid more.
    local active = {}
    for _, name in ipairs(snap.buffs or {}) do
        table.insert(active, name)
    end
    table.sort(active)
    local key    = table.concat({ roster(floor), snap.self.hp, snap.self.mp or 0, table.concat(active, "+") }, "|")
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

-- One round of a plan's floors before servicing: each floor's stock (the exp of clearing it once) at its combat rate,
-- not the ranking's cycle that waits on respawn. Supply is sized to this clear (supply.plan); the wait has no fight
-- drain. A lasting gain is weighed over the clear, not until the next level: exp is exp at any level, and at 99 the
-- time to a level that never comes made a gear's worth infinite and its value NaN, so the gear was never bought and
-- kept a trio from inviting for 10 minutes (offline 20261115-104514; user 11-15).
function M.lap(plan)
    if plan == nil then
        return 0
    end
    if plan.lap ~= nil then
        return plan.lap
    end
    local time = 0
    for _, floor in ipairs(plan.floors or {}) do
        if (floor.rate or 0) > 0 then
            time = time + (floor.stock or 0) / floor.rate
        end
    end
    return time
end

-- What is left of the lap since the last servicing; a bot that never serviced has it all ahead.
function M.lap_left(snap, plan)
    local lap    = M.lap(plan)
    local served = snap.record ~= nil and snap.record.memory.serviced or nil
    if served == nil then
        return lap
    end
    return math.max(0, lap - (snap.now - served) / 1000)
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
local regions = { count = 0 }
local sweeps  = {}

local function slot(snap, crew)
    return (snap.self.class or "") ..
        (snap.self.level or 0) .. (snap.hypothesis or "") .. (crew ~= nil and crew.key or "")
end

local function reach(snap, query, crew)
    snap          = snap.actual or snap
    local shunned = danger.shunned(snap, query)
    local traits  = table.concat({ snap.self.level, score.signature(snap), table.concat(shunned, ",") }, "|")
    local base    = snap.world.map .. "|" .. traits
    -- Reach depends on which exits are walkable from where the bot stands, and those are the same from every
    -- tile of one walkable region of the map (atlas::reach searches by map and region), so the region is part
    -- of the key; a tile of no region (a warp tile, a wall) keeps an entry of its own. Callers key their own
    -- caches on the reached set (regions token), so moving inside one set does not re-rank grounds. A map-only
    -- key reused another spot's result and made the group plan vanish for a moment (plan_lost -> member parted);
    -- a key by tile asked again at every step, a tenth of decision time.
    local owner   = slot(snap, crew)
    local region  = query.region(snap.world.map, snap.world.x, snap.world.y)
    local spot    = region ~= 0 and region or tostring(snap.world.x) .. "," .. tostring(snap.world.y)
    local found   = memo.peek(reaches, owner, base, spot)
    if found == nil then
        local set = query.reach(danger.perilous(snap, query, crew or false), shunned,
            { map = snap.world.map, x = snap.world.x, y = snap.world.y, level = snap.self.level })
        -- A failed query (nil) means "unknown", not "nowhere to go": it is not cached. Caching it as an
        -- empty set once froze low-level solo bots for minutes beside dozens of mobs, because every later
        -- ranking at that tile saw zero reachable grounds.
        if set == nil then
            return {}, base .. "|unknown", traits .. "|unknown"
        end
        local ids = {}
        for map in pairs(set) do
            table.insert(ids, map)
        end
        table.sort(ids)
        local list = table.concat(ids, ",")
        if regions[list] == nil then
            regions.count = regions.count + 1
            regions[list] = regions.count
        end
        found = memo.get(reaches, owner, base, spot, function()
            return { set = set, key = base .. "|" .. regions[list], area = traits .. "|" .. regions[list] }
        end)
    end
    return found.set, found.key, found.area
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

local function peaks(floors)
    local worst = 0
    for _, floor in ipairs(floors) do
        worst = math.max(worst, floor.peak or 0)
    end
    return worst
end

-- What a lap spends and brings in a second: each floor's mana, wounds and loot weighed by the time it is cleared in
-- (M.lap), so the supply bought for a lap and the bag it fills are sized on the whole lap. Taken from its last floor, a
-- lap of 흑해골굴 three floors deep filled the bag with loot in over a day and one nine floors deep in under three
-- minutes, and the supply a warrior walked to the shop for was not wanted there (online 061552).
local function over(floors)
    local time   = 0
    local drain  = 0
    local wear   = 0
    local loot   = 0
    local drains = {}
    local yields = {}
    for _, floor in ipairs(floors) do
        local stay = (floor.rate or 0) > 0 and (floor.stock or 0) / floor.rate or 0
        time       = time + stay
        drain      = drain + stay * (floor.drain or 0)
        wear       = wear + stay * (floor.wear or 0)
        loot       = loot + stay * (floor.loot or 0)
        for name, rate in pairs(floor.drains or {}) do
            drains[name] = (drains[name] or 0) + stay * rate
        end
        for name, entry in pairs(floor.yields or {}) do
            local found = yields[name] or { rate = 0, capacity = entry.capacity }
            found.rate = found.rate + stay * entry.rate
            yields[name] = found
        end
    end
    if time <= 0 then
        local last = floors[#floors] or {}
        return { drain = last.drain, drains = last.drains, wear = last.wear, loot = last.loot, yields = last.yields }
    end
    for name, rate in pairs(drains) do
        drains[name] = rate / time
    end
    for _, entry in pairs(yields) do
        entry.rate = entry.rate / time
    end
    return { drain = drain / time, drains = drains, wear = wear / time, loot = loot / time, yields = yields }
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

local grounds = nil

-- Two maps are on the same ground when one hunting bundle lists both as floors. roster_of keys a single
-- floor by its mobs, so it cannot tell that two floors belong to the same ground.
function M.same_ground(left, right)
    if grounds == nil then
        grounds = {}
        for index, bundle in ipairs(hunting.bundles) do
            for _, floor in ipairs(bundle.floors) do
                grounds[floor.map] = grounds[floor.map] or {}
                grounds[floor.map][index] = true
            end
        end
    end
    for index in pairs(grounds[left] or {}) do
        if (grounds[right] or {})[index] then
            return true
        end
    end
    return false
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

local function avoided(snap, crew)
    local found = {}
    local keys  = {}
    local maps  = {}
    local team  = { snap.actual or snap }
    for _, member in ipairs(crew ~= nil and crew.members or {}) do
        table.insert(team, member)
    end
    for _, member in ipairs(team) do
        local power = danger.power(member)
        for key, entry in pairs(((member.record or {}).memory or {}).fled or {}) do
            if type(key) == "string" and key:sub(1, 2) == "r:" and type(entry) == "table" and entry.power == power and
                (entry.strikes or 0) > 1 and found[key:sub(3)] == nil then
                found[key:sub(3)] = true
                table.insert(keys, key .. "@" .. tostring(entry.map))
                table.insert(maps, entry.map)
            end
        end
    end
    table.sort(keys)
    return found, table.concat(keys, ","), maps
end

local function shunned_bundle(bundle, avoid)
    for _, floor in ipairs(bundle.floors) do
        if avoid[roster(floor)] then
            return true
        end
    end
    return false
end

candidates = function(snap, query, mode, crew, open)
    local known, _, area     = reach(snap, query, crew)
    local owner              = slot(snap, crew)
    local base               = score.signature(snap, "")
    local signature          = score.signature(snap, mode)
    local avoid, marks, maps = avoided(snap, crew)
    if open then
        avoid, marks, maps = {}, "", {}
    end
    -- Kept by the maps in reach, not by the map stood on: the grounds worth a sweep are the same from every
    -- map that reaches the same ones, and sweeping them all again at each new floor cost a tenth of a second.
    local listed = memo.get(cache, owner, base, signature .. "|" .. area .. "|" .. marks, function()
        local rested  = score.rested(snap, mode)
        local ability = score.capability(rested, query)
        local limit   = math.huge
        local riskier = {}
        for _, map in ipairs(maps) do
            local floor = raw(map)
            if floor ~= nil then
                limit = math.min(limit, M.evaluate(rested, query, floor, ability, crew).peak)
            end
        end
        local found  = {}
        local wanted = snap.actual ~= nil and nearby(snap.actual, query, mode, crew) or nil
        for index = bound(rested), 1, -1 do
            local bundle = hunting.bundles[index]
            if known[bundle.floors[1].map] and (wanted == nil or wanted[index]) and shunned_bundle(bundle, avoid) == false then
                local floors = memo.get(sweeps, owner, base, signature .. "|" .. index, function()
                    return sweep(rested, query, bundle, ability, crew)
                end)
                if #floors > 0 and peaks(floors) < limit then
                    table.insert(found, { bundle = bundle, floors = floors })
                elseif #floors > 0 then
                    table.insert(riskier, { bundle = bundle, floors = floors, peak = peaks(floors) })
                end
            end
            if #found >= combat.candidates then
                break
            end
        end
        if #found == 0 and #riskier > 0 then
            local lowest = math.huge
            for _, entry in ipairs(riskier) do
                lowest = math.min(lowest, entry.peak)
            end
            for _, entry in ipairs(riskier) do
                if entry.peak <= lowest then
                    table.insert(found, { bundle = entry.bundle, floors = entry.floors })
                end
            end
        end
        return found
    end)
    if #listed == 0 and marks ~= "" then
        return candidates(snap, query, mode, crew, true)
    end
    return listed
end

local function remaining(snap, floor)
    local dry = (((snap.actual or snap).record or {}).memory or {}).dry
    local at  = dry ~= nil and dry[tostring(floor.map)] or nil
    if at == nil or (floor.mobs or 0) <= 0 then
        return 1
    end
    return 1 - math.exp(-math.max(0, (snap.now or at) - at) / 1000 * floor.supply / floor.mobs)
end

-- A floor is worth the mobs standing on it, cleared once a lap: what respawns while I clear it is not counted, and a
-- floor my last lap emptied counts only what has come back since (remaining), so a lap that outran the respawn goes on
-- to another ground and one the respawn has refilled is a candidate again (user 11-16). Counted at the respawn, the
-- kills a crew makes past it were lost and every ground of a 99 crew of four came out within a tenth of the others
-- (brain unit probe, 63k-77k). Fewer mobs stand further apart, so the walk to each is stretched by how much thinner
-- they stand.
local function cleared(snap, floor, recovering)
    local left = recovering and remaining(snap, floor) or 1
    if (floor.rate or 0) <= 0 or (floor.stock or 0) <= 0 or left <= 0 then
        return 0, 0
    end
    local whole = (floor.kills or 0) > 0 and 1000 / floor.kills or 0
    local walk  = math.min(floor.walk or 0, whole)
    local pace  = whole > 0 and whole / (whole - walk + walk / math.sqrt(left)) or 1
    local stock = floor.stock * left
    return stock, stock / (floor.rate * pace)
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
    -- Level is in the key for the same reason as restock: warps between floors have level limits.
    local key = floors[1].map .. ">" .. floors[2].map .. "@" .. (snap.self.level or 0)
    if hops[key] == nil then
        local spots = query.spawns(floors[1].map, floors[1].targets)
        local spot  = spots ~= nil and spots[1] or nil
        local route = spot ~= nil and query.path({ maps = { floors[2].map } }, {}, {},
            { map = floors[1].map, x = spot.x, y = spot.y, level = snap.self.level }) or nil
        hops[key]   = route ~= nil and route.cost * (snap.pace.step or combat.walk) / 1000 or math.huge
    end
    return hops[key]
end

-- A ground somebody else hunts, alone or in a group, is theirs first: a group laps its floors in order, so another
-- group on the same ground meets it floor by floor and takes from the standing stock of that lap. Of a floor I count
-- only the mobs their kills leave while I clear it. Split by respawn rate instead, the share disagreed with the lap
-- the ranking weighs, and a second group still went where the ground was worth twice the rest, and both lost: 12.4M
-- and 17.9M on 흑해골굴 against 22.4M and 25.0M alone (online 203032 against 200750; user 11-15: a ground somebody holds
-- is not entered to take from them, while beginners weigh the walk to an empty ground against what a crowded one has
-- left). The share is taken where a ground is picked, not in the ranking a period keeps: kept in it, three groups that
-- chose within one period all saw 흑해골굴 empty and went there (online 170334, 17:05:11-41).
local function held(query, floors, crew)
    local wanted = 0
    local left   = 0
    for _, floor in ipairs(floors) do
        local mobs = floor.mobs or 0
        local stay = (floor.rate or 0) > 0 and (floor.stock or 0) / floor.rate or 0
        local take = query.demand(floor.map, crew ~= nil and crew.names or nil) * stay
        wanted     = wanted + mobs
        left       = left + math.max(0, mobs - take)
    end
    return wanted > 0 and left / wanted or 1
end

-- The walk between a floor and the town a bot services in, one way; a way not known is no walk to weigh, as an
-- unknown reach is not "nowhere" (reach).
local function homeward(snap, query, floor)
    local trip = restock(snap, query, floor)
    return trip < math.huge and trip / 2000 or 0
end

-- The walk from a floor to town and back that a servicing takes, in seconds.
function M.servicing(snap, query, floor)
    return 2 * homeward(snap, query, floor)
end

-- The mana a bag carries into a lap, for a bot that gets its mana back by drinking alone: the doses that fill it. One
-- that charges (공력증강) gets it back on the floor, and no bag bounds its lap.
local function bagged(member)
    if recovery.spells(member).charge ~= nil then
        return math.huge
    end
    local slot = recovery.carriable(member, "mp")
    local free = ((member.bag or {}).capacity or 0) - 2
    return slot > 0 and free > 0 and slot * free or math.huge
end

-- A lap runs from one servicing to the next: the walk from town to its first floor and back, and the floors cleared
-- once, so a far ground pays its walk on every lap and a crowded ground near town can still beat an empty one far from
-- it (user 11-15: beginners). A lap is left by a jump (leave_ground means), so the way home is weighed as the way in:
-- asked of every floor, the walks home took 36s of a crew of four's plan against 11 (brain unit probe, 3rd promotion
-- crew at 흉노족1; online 015836, plans of 45-57s while the crew stood). How deep the lap goes is chosen with the walk in
-- it, as it is made once for the whole lap: chosen before it, a lap of floors alike in worth stopped at its first floor
-- and carried the whole walk on it, and 국내백륜동 fell from 77k to 63k for a 99 crew of four (brain unit probe). Weighed
-- from where the bot stood instead, the lap was three floors deep standing on 흑해골굴 and nine from town, the supply sized
-- for one lap found nothing to buy for the other, and a warrior walked to the shop and back empty three times (online
-- 061552, 06:22-06:24). The supply of a lap is bought whole at its servicing (supply.plan), so a lap goes no deeper
-- than the doses of any member's bag last: past them a member walks out for more halfway round. Ranked by clear and
-- town alone, a 99 crew of four still preferred weak multi-floor caves: their clear was near nothing, so the score was
-- stock over the walk, while a stronger ground cut short by safety carried less stock (user 11-16). A crew that stays
-- on one ground waits for the first floor to refill before the next lap; that wait caps the rate without counting
-- mid-lap respawn kills (which once packed every ground of the crew into a tenth of each other). The bag and M.lap keep
-- the clear only: wait has no fight drain.
local function lapped(snap, query, candidate, worth, moving, crew, recovering)
    local finish = 1
    local rate   = 0
    local best   = 0
    local spent  = 0
    local stock  = 0
    local first  = 0
    local refill = 0
    local gap    = moving and hop(snap, query, candidate.floors) or 0
    local out    = homeward(snap, query, candidate.floors[1])
    local bags   = { { bag = bagged(snap), regen = recovery.rates(snap).mp, need = 0 } }
    for _, member in ipairs(crew ~= nil and crew.members or {}) do
        table.insert(bags,
            { name = member.self.name, bag = bagged(member), regen = recovery.rates(member).mp, need = 0 })
    end
    for index, floor in ipairs(candidate.floors) do
        local left      = recovering and remaining(snap, floor) or 1
        local exp, time = cleared(snap, floor, recovering)
        local carried   = true
        for _, entry in ipairs(bags) do
            local drain = entry.name == nil and floor.drain or (floor.drains or {})[entry.name] or entry.regen
            entry.need  = entry.need + math.max(0, (drain or 0) - entry.regen) * time
            carried     = carried and entry.need <= entry.bag
        end
        if carried == false and index > 1 then
            break
        end
        if index == 1 then
            first  = time
            refill = (floor.supply or 0) > 0 and (floor.mobs or 0) > 0 and left * floor.mobs / floor.supply or 0
        end
        stock        = stock + exp
        spent        = spent + time
        local took   = spent + (index > 1 and index * gap or 0)
        local trip   = 2 * out
        local wait   = math.max(0, refill - (took - first + trip))
        local cycle  = took + trip + wait
        local open   = took > 0 and cycle > 0 and cycle < math.huge
        local combat = open and stock / took or 0
        local sum    = open and (combat + bonus(floor, worth)) * took / cycle or 0
        if sum > best then
            best   = sum
            rate   = stock / cycle
            finish = index
        end
    end
    return finish, rate, best
end

-- What a plan is worth when a rival hunts the same floors beside it: the rival's kills join the others' demand on
-- every floor both plans hold, and the plan keeps its share of the kills there (lapped). Plans on other floors keep
-- theirs.
function M.contended(snap, query, plan, rival)
    if plan == nil then
        return 0
    end
    local value = plan.top or plan.rate
    local taken = {}
    local both  = false
    for _, floor in ipairs(rival ~= nil and rival.floors or {}) do
        taken[floor.map] = floor.kills or 0
    end
    for _, floor in ipairs(plan.floors) do
        both = both or taken[floor.map] ~= nil
    end
    if both == false then
        return value
    end
    local shared  = setmetatable({
        demand = function(map, crew)
            return query.demand(map, crew) + (taken[map] or 0)
        end
    }, { __index = query })
    local _, rate = lapped(snap, shared, { floors = plan.floors }, {}, false, nil, true)
    return math.min(value, rate * held(shared, plan.floors, nil))
end

local function ordered(ranked)
    table.sort(ranked, function(left, right)
        if left.score ~= right.score then
            return left.score > right.score
        end
        return left.candidate.bundle.name < right.candidate.bundle.name
    end)
end

local function settle(snap, query, ranked, worth, crew, recovering)
    ordered(ranked)
    local settled = 0
    for index, entry in ipairs(ranked) do
        if entry.score < settled then
            break
        end
        if entry.finish > 1 and entry.moving ~= true then
            local finish, rate, score = lapped(snap, query, entry.candidate, worth, true, crew, recovering)
            entry = { candidate = entry.candidate, finish = finish, rate = rate, score = score, moving = true }
            ranked[index] = entry
        end
        settled = math.max(settled, entry.score)
    end
    ordered(ranked)
end

local function nearest_first(snap, query, ranked)
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
end

local function rank(snap, query, mode, crew, worth)
    local ranked = {}
    for _, candidate in ipairs(candidates(snap, query, mode, crew)) do
        local finish, rate, best = lapped(snap, query, candidate, worth, false, crew)
        if best > 0 then
            table.insert(ranked, { candidate = candidate, finish = finish, rate = rate, score = best })
        end
    end
    settle(snap, query, ranked, worth, crew, false)
    nearest_first(snap, query, ranked)
    return ranked
end

local function dried(snap, candidate)
    for _, floor in ipairs(candidate.floors) do
        if remaining(snap, floor) < 1 then
            return true
        end
    end
    return false
end

-- A floor I emptied comes back with its respawn, which goes on with time, while the ranking stays the same until what
-- it is made from changes (ranked). So the regrowth is weighed where a ground is picked, on the grounds that have such
-- a floor, and the list is ordered again by the same steps as the ranking.
local function recovered(snap, query, ranked, crew)
    local found = nil
    for index, entry in ipairs(ranked) do
        if found == nil and dried(snap, entry.candidate) then
            found = {}
            for before = 1, index - 1 do
                found[before] = ranked[before]
            end
        end
        if found ~= nil and dried(snap, entry.candidate) then
            local finish, rate, score = lapped(snap, query, entry.candidate, ranked.worth, false, crew, true)
            if score > 0 then
                table.insert(found, { candidate = entry.candidate, finish = finish, rate = rate, score = score })
            end
        elseif found ~= nil then
            table.insert(found, entry)
        end
    end
    if found == nil then
        return ranked, ""
    end
    settle(snap, query, found, ranked.worth, crew, true)
    nearest_first(snap, query, found)
    local before = {}
    for _, entry in ipairs(ranked) do
        before[entry.candidate] = entry.score
    end
    local marks = {}
    for _, entry in ipairs(found) do
        table.insert(marks, string.format("%s~%d:%.2f", entry.candidate.bundle.name, entry.finish,
            entry.score / before[entry.candidate]))
    end
    return found, table.concat(marks, ",")
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
local lasting   = {}

local function alike(left, right)
    if #left ~= #right then
        return false
    end
    for index, entry in ipairs(left) do
        local other = right[index]
        if entry.candidate ~= other.candidate or entry.finish ~= other.finish or entry.rate ~= other.rate or
            entry.score ~= other.score then
            return false
        end
    end
    return true
end

function M.emptied(snap)
    local latest = 0
    for _, at in pairs(((snap.actual or snap).record.memory or {}).dry or {}) do
        latest = math.max(latest, at)
    end
    return latest
end

function M.ranked(snap, query, mode, crew)
    local _, context = reach(snap, query, crew)
    local _, marks   = avoided(snap, crew)
    local region     = snap.world.x ~= nil and query.region(snap.world.map, snap.world.x, snap.world.y) or 0
    local spot       = region ~= 0 and region or tostring(snap.world.x) .. "," .. tostring(snap.world.y)
    local aim        = ((snap.actual or snap).record.memory or {}).aim
    local base       = score.signature(snap, "")
    local key        = table.concat({ base, score.signature(snap, mode), context, marks, snap.world.map, spot,
        tostring(aim) }, "|")
    local owner      = slot(snap, crew)
    -- Always rank for this exact key. A per-decision budget used to hand back the last ranking computed
    -- for another state; that stale list produced an empty plan, party.plan cached it, and low-level solo
    -- bots stood still beside dozens of mobs. Decisions must depend on the state, never on call history.
    -- The key holds all a ranking is made from: the walk to each ground starts in the region stood in, the ground
    -- aimed at goes first among equals, and what changes with time alone (a floor's regrowth, what others take) is
    -- weighed where a ground is picked. Ranked again every five seconds as well, the rankings spent 55 of their 122
    -- seconds at those turns while a mage stood four seconds under skeletons in one decision (online 215552).
    return memo.get(standings, owner, base, key, function()
        local ranked = rank(snap, query, mode, crew, {})
        local worth  = materials(snap, query, mode, crew, ranked[1] ~= nil and ranked[1].rate or 0)
        if next(worth) ~= nil then
            ranked = rank(snap, query, mode, crew, worth)
        end
        local kept = memo.keep(lasting, owner, base, key, ranked, alike)
        kept.worth = worth
        return kept
    end)
end

function M.standing(snap, query, crew)
    return (recovered(snap, query, M.ranked(snap, query, nil, crew), crew))
end

function M.reachable(snap, query, candidate, crew)
    return reach(snap, query, crew)[candidate.floors[1].map] == true
end

function M.floor_at(map)
    return raw(map)
end

function M.roster_of(map)
    local floor = raw(map)
    return floor ~= nil and roster(floor) or nil
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

local function upper_mean(values)
    local total = 0
    for _, entry in ipairs(values) do
        total = total + entry
    end
    local mean  = #values > 0 and total / #values or 0
    local above = 0
    local count = 0
    for _, entry in ipairs(values) do
        if entry >= mean then
            above = above + entry
            count = count + 1
        end
    end
    return count > 0 and above / count or 0
end

local function density_of(floors)
    local total = 0
    for _, floor in ipairs(floors) do
        total = total + (floor.density or 0)
    end
    return #floors > 0 and total / #floors or 0
end

-- The plan that makes the most a second is hunted, and the denser floors break a tie only: the rate already counts the
-- walk between mobs that a thin floor costs. Chosen by density among every plan above the mean, a 99 crew of four went
-- to 평양흑해골굴 at 71k past the haunted houses at 77k (brain unit test; user 11-17: the score first). The best rate is
-- kept as top, apart from peak, the burst a plan's floors deal that resting is weighed by (rest.volleys): written over
-- peak, a 3rd promotion crew's 1.3M a second stood in for its blows, so any member below a full bar counted as low
-- (brain unit probe at 흉노족1; online 001330, a 검황 ate 팔과탕 for 1,000 hp between its 백호참).
local function prefer(plans)
    local best = nil
    for _, plan in ipairs(plans) do
        if best == nil or plan.rate > best.rate or (plan.rate == best.rate and plan.density > best.density) then
            best = plan
        end
    end
    if best ~= nil then
        best.top = best.rate
    end
    return best
end

local function available(snap, query, ranked, shunned, crew, guards)
    local options = {}
    local rates   = {}
    for _, entry in ipairs(ranked) do
        table.insert(rates, entry.rate)
    end
    local cut = math.min(upper_mean(rates), rates[1] or 0)
    for _, entry in ipairs(ranked) do
        if entry.rate < cut and #options > 0 then
            break
        end
        local candidate = entry.candidate
        local floors    = {}
        for index = 1, entry.finish do
            if shunned[candidate.floors[index].map] then
                break
            end
            table.insert(floors, candidate.floors[index])
        end
        floors = guarded(floors, query, guards)
        local open = #floors > 0 and M.reachable(snap, query, candidate, crew)
        for _, guard in ipairs(guards or {}) do
            open = open and M.reachable(guard.rested, query, candidate, guard.crew)
        end
        if open then
            local lap = over(floors)
            table.insert(options, {
                name    = candidate.bundle.name,
                floors  = floors,
                density = density_of(floors),
                rate    = entry.rate,
                need    = floors[#floors].need,
                margin  = floors[#floors].margin,
                drain   = lap.drain,
                drains  = lap.drains,
                spend   = floors[#floors].spend,
                wear    = lap.wear,
                pain    = floors[#floors].pain,
                press   = floors[#floors].press,
                peak    = peaks(floors),
                loot    = lap.loot,
                yields  = lap.yields,
                deaths  = floors[#floors].deaths,
                crowd   = floors[#floors].crowd,
            })
        end
    end
    return options
end

local picked = setmetatable({}, { __mode = "k" })

local function fighter(snap, query, crew)
    local active = {}
    for _, name in ipairs(snap.buffs or {}) do
        table.insert(active, name)
    end
    table.sort(active)
    local _, context = reach(snap, query, crew)
    return table.concat({ snap.self.name or "", snap.hypothesis or "", crew ~= nil and crew.key or "",
        score.signature(snap), snap.self.hp, snap.self.mp or 0, table.concat(active, "+"), context }, "|")
end

-- The grounds open to a list depend on the list, on where each fighter can walk and what it shuns, and on how
-- each guard fares on a floor (the key of evaluate): while none of those moved they are the ones found before.
-- Walking the list again for every decision of every crewmate cost more than the ranking itself.
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
    local ranked           = M.ranked(snap, query, mode, crew)
    local listed, regrowth = recovered(snap, query, ranked, crew)
    local _, context       = reach(snap, query, crew)
    local parts            = { context, regrowth }
    local shares           = {}
    local open             = listed
    for index, entry in ipairs(listed) do
        local kept = held(query, entry.candidate.floors, crew)
        if kept < 1 then
            table.insert(shares, index .. ":" .. string.format("%.2f", kept))
        end
        if kept < 1 and open == listed then
            open = {}
            for before = 1, index - 1 do
                table.insert(open, listed[before])
            end
        end
        if open ~= listed then
            table.insert(open, {
                candidate = entry.candidate,
                finish = entry.finish,
                rate = entry.rate * kept,
                score = entry.score * kept
            })
        end
    end
    if open ~= listed then
        ordered(open)
    end
    table.insert(parts, table.concat(shares, ","))
    for _, guard in ipairs(guards or {}) do
        table.insert(parts, fighter(guard.rested, query, guard.crew))
    end
    local maps = {}
    for map in pairs(shunned) do
        table.insert(maps, map)
    end
    table.sort(maps)
    table.insert(parts, table.concat(maps, ","))
    local key   = table.concat(parts, "#")
    local known = picked[ranked]
    if known == nil then
        known          = {}
        picked[ranked] = known
    end
    if known[key] == nil then
        known[key] = available(snap, query, open, shunned, crew, guards)
    end
    local options = {}
    for index, option in ipairs(known[key]) do
        local copy = {}
        for name, value in pairs(option) do
            copy[name] = value
        end
        options[index] = copy
    end
    local chosen = prefer(options)
    if chosen ~= nil then
        chosen.options = options
    end
    return chosen
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
        local made  = mode == nil and crew == nil and guards == nil and snap.hypothesis == nil and
            require("brain.policies.party").pin(snap) == nil and require("brain.planner").current(snap, query) or nil
        local found = nil
        if made ~= nil then
            found = made.chosen
        elseif mode == nil and recovery.rates(snap).mp <= 0 and #recovery.doses(snap, "mp") == 0 then
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

-- The mana a second this bot spends on a plan: its own when the plan has it among the crew, else the plan's, which
-- is the leader's own and so the bot's when it leads or hunts alone.
function M.drain(plan, snap)
    local own = plan.drains ~= nil and plan.drains[snap.self.name] or nil
    return own or plan.drain
end

-- A mode is the state of the member asking (members[1]): its plan with its mana refilled against one without. Set on
-- whoever leads instead, a warrior weighing its mana potions compared its own refilled plan with its poet's dry one
-- and skipped its first supply (online 105006, 111611; plans logged in 112645).
function M.party(members, query, mode)
    local function state(member)
        if member == members[1] and mode ~= "SUPPLY" then
            return mode
        end
        return M.mode(member)
    end
    local known = { rested = {}, ability = {}, crew = {} }
    local function rested(member)
        if known.rested[member] == nil then
            known.rested[member] = score.rested(member, state(member))
        end
        return known.rested[member]
    end
    local function crew(list)
        local names = {}
        local found = {}
        for _, member in ipairs(list) do
            table.insert(names, member.self.name)
            table.insert(found, rested(member))
        end
        local key = table.concat(names, ",")
        if known.crew[key] == nil then
            known.crew[key] = M.crew(found, query)
        end
        return known.crew[key]
    end
    local best   = nil
    local front  = 1
    local merged = {}
    local owners = {}
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
            if known.ability[member] == nil then
                known.ability[member] = score.capability(rested(member), query)
            end
            table.insert(guards, { rested = rested(member), ability = known.ability[member], crew = crew(rest) })
        end
        local plan = M.choose(lead, query, state(lead),
            #others > 0 and crew(others) or nil, #guards > 0 and guards or nil)
        for _, option in ipairs(plan ~= nil and plan.options or {}) do
            table.insert(merged, option)
            owners[option] = index
        end
    end
    best = prefer(merged)
    if best ~= nil then
        front = owners[best]
    end
    return best, front
end

local function held(members, index, query, bundle, known)
    local function rested(at)
        if known.rested[at] == nil then
            known.rested[at] = score.rested(members[at], M.mode(members[at]))
        end
        return known.rested[at]
    end
    local function ability(at)
        if known.ability[at] == nil then
            known.ability[at] = score.capability(rested(at), query)
        end
        return known.ability[at]
    end
    local function crew(list)
        local key = table.concat(list, ",")
        if known.crew[key] == nil then
            local found = {}
            for _, at in ipairs(list) do
                table.insert(found, rested(at))
            end
            known.crew[key] = M.crew(found, query)
        end
        return known.crew[key]
    end
    local lead   = rested(index)
    local others = {}
    for at = 1, #members do
        if at ~= index then
            table.insert(others, at)
        end
    end
    local guards = {}
    for slot, at in ipairs(others) do
        local rest = { index }
        for other, entry in ipairs(others) do
            if other ~= slot then
                table.insert(rest, entry)
            end
        end
        table.insert(guards, { rested = rested(at), ability = ability(at), crew = crew(rest) })
    end
    local shunned = {}
    for _, member in ipairs(members) do
        for _, map in ipairs(danger.shunned(member, query)) do
            shunned[map] = true
        end
    end
    local floors = {}
    for _, floor in ipairs(sweep(lead, query, bundle, ability(index), #others > 0 and crew(others) or nil)) do
        if shunned[floor.map] then
            break
        end
        table.insert(floors, floor)
    end
    return guarded(floors, query, guards)
end

local function planned(bundle, floors)
    if #floors == 0 then
        return nil
    end
    local rate = 0
    for _, floor in ipairs(floors) do
        rate = math.max(rate, floor.rate)
    end
    local last = floors[#floors]
    local lap  = over(floors)
    return {
        name   = bundle.name,
        floors = floors,
        rate   = rate,
        need   = last.need,
        margin = last.margin,
        drain  = lap.drain,
        drains = lap.drains,
        spend  = last.spend,
        wear   = lap.wear,
        pain   = last.pain,
        press  = last.press,
        peak   = peaks(floors),
        loot   = lap.loot,
        yields = lap.yields,
        deaths = last.deaths,
        crowd  = last.crowd,
    }
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
    local others = {}
    for index = 2, #members do
        table.insert(others, members[index])
    end
    local avoid = avoided(members[1], #others > 0 and { members = others } or nil)
    if shunned_bundle(bundle, avoid) then
        return nil
    end
    for _, member in ipairs(members) do
        if M.reachable(member, query, bundle) == false then
            return nil
        end
    end
    local floors = {}
    local known  = { rested = {}, ability = {}, crew = {} }
    for index = 1, #members do
        local found = held(members, index, query, bundle, known)
        if #found > #floors then
            floors = found
        end
    end
    return planned(bundle, floors)
end

local function on_bundle(snap, bundle)
    for _, floor in ipairs(bundle.floors) do
        if snap.world.map == floor.map then
            return true
        end
    end
    return false
end

function M.force(members, query, name)
    local bundle = nil
    for _, entry in ipairs(hunting.bundles) do
        if entry.name == name then
            bundle = entry
        end
    end
    if bundle == nil or #members == 0 then
        return nil
    end
    local lead    = members[1]
    local rested  = score.rested(lead)
    local ability = score.capability(rested, query)
    local others  = {}
    for index = 2, #members do
        table.insert(others, members[index])
    end
    local crew   = #others > 0 and M.crew(others, query) or nil
    local floors = sweep(rested, query, bundle, ability, crew)
    if #floors == 0 then
        local result = M.evaluate(rested, query, bundle.floors[1], ability, crew)
        floors = { result }
    end
    return planned(bundle, floors)
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
