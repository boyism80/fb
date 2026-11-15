local data     = require("brain.data.combat")
local magic    = require("brain.data.spells")
local battle   = require("brain.policies.battle")
local combat   = require("brain.policies.combat")
local escort   = require("brain.policies.escort")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local value    = require("brain.policies.value")

local M = {}

local plans = setmetatable({}, { __mode = "k" })

local function copy(source, changes)
    local result = {}
    for key, entry in pairs(source) do
        result[key] = entry
    end
    for key, entry in pairs(changes) do
        result[key] = entry
    end
    return result
end

function M.risk(snap, member, current)
    local blows = combat.pressure(snap, member, 2 * combat.cast(snap))
    return blows > 0 and math.exp(-current / blows) or 0
end

local function rescue(snap, query, member, current, restored, heal, worth)
    if heal.hp <= 0 then
        return 0
    end
    local blows = combat.pressure(snap, member, 2 * combat.cast(snap))
    if blows <= 0 then
        return 0
    end
    return (math.exp(-current / blows) - math.exp(-(current + restored) / blows)) *
        party.death(snap, query, member, worth)
end

local function volley(snap, member)
    local attackers = {}
    for _, mob in ipairs(member.world.map == snap.world.map and member.world.x ~= nil and snap.world.mobs or {}) do
        if mob.x ~= nil and combat.hits(mob) and
            math.abs(mob.x - member.world.x) + math.abs(mob.y - member.world.y) == 1 then
            table.insert(attackers, { mob = mob, count = 1 })
        end
    end
    return #attackers > 0 and combat.imminent(member, attackers) or 0
end

-- A crewmate, or I, falls short when the mobs on it drain more hp before it ends them itself than it can spare above
-- two of their volleys, and then does not live through the fight, so each hp of that shortfall a heal restores or a
-- held mob spares is worth its share of that death, as the death within two casts is (rescue). Weighed by those two
-- casts alone, a warrior at 1,933 hp with no mana for 건곤대나이 under weak blows looked safe, its mage poisoned twenty
-- mobs in 11s instead of healing it, and both left (online WA+MA 160559, 16:14:09-20; user 11-15: keep the crew alive
-- first, the poison can wait).
local function leaving(snap, query, member, current, worth)
    local peril = member == snap and party.peril(snap, query) or member.peril
    if type(peril) ~= "table" or (peril.drain or 0) <= 0 or (peril.hp or 0) <= 0 then
        return nil
    end
    local clear = peril.hp / math.max(peril.dps or 0, 1e-9)
    local short = peril.drain * clear - (current - 2 * volley(snap, member))
    if short <= 0 then
        return nil
    end
    return {
        short  = short,
        clear  = clear,
        drains = peril.drains or {},
        cost   = party.death(snap, query, member, worth),
    }
end

-- A crewmate in sight has the lower of the bar over its head and the hp it posts of itself: the server sends no bar
-- for hp a spell spends, and read from the bar alone a mage that 공력증강 left at 100 looked whole to its poet, which
-- healed only the warrior for 17s until the mage died (online WA+PO+MA 150824, 15:12:10-27). The posted hp in turn
-- lags a blow the bar already shows.
local function standing(member, ally)
    local bar = ally ~= nil and ally.percent ~= nil and (member.self.max_hp or 0) * ally.percent / 100 or nil
    if bar == nil then
        return member.self.hp or 0
    end
    return math.min(bar, member.self.hp or bar)
end

local function heals(snap, query, slots, worth)
    local found   = {}
    local mana    = value.spend(snap, query, nil)
    local reserve = recovery.draining(snap) and 0 or recovery.spare(snap)
    local seen    = value.targets(snap)
    for _, member in ipairs(party.members(snap)) do
        local ally = query.ally(member.self.name)
        if ally == nil and member.self.ghost == false and member.world.map == snap.world.map and member.world.x ~= nil and
            (member.self.hp or 0) < (member.self.max_hp or 0) then
            local best = 0
            for _, heal in ipairs(combat.options(snap, {}).heal) do
                if heal.others and slots[heal.name] ~= nil then
                    best = math.max(best, math.min(heal.hp, member.self.max_hp - member.self.hp))
                end
            end
            local steps = math.abs(member.world.x - snap.world.x) + math.abs(member.world.y - snap.world.y)
            if best > 0 and steps > 0 then
                local hunted  = member.battle
                local target  = type(hunted) == "table" and seen[hunted.oid] or nil
                local fuel    = target ~= nil and value.convertible(member, target) or 0
                local averted = rescue(snap, query, member, member.self.hp, best, { hp = best }, worth)
                table.insert(found, {
                    urgent = combat.pressure(snap, member, combat.cast(snap)) >= member.self.hp,
                    walk = { x = member.world.x, y = member.world.y },
                    averted = averted,
                    value = best * fuel + averted - steps * (snap.pace.step or data.walk) / 1000 * worth.value,
                })
            end
        end
        local current = standing(member, ally)
        local deficit = (member.self.max_hp or 0) - current
        local memory  = snap.record.memory.battle
        local dying   = 2 * volley(snap, member) >= current
        if ally ~= nil and deficit > 0 and member.self.ghost == false then
            for _, heal in ipairs(combat.options(snap, {}).heal) do
                local refused = memory ~= nil and memory.oid == ally.oid and memory.refused == heal.name
                if heal.others and slots[heal.name] ~= nil and heal.mp + (dying and 0 or reserve) <= (snap.self.mp or 0) and refused == false then
                    local restored = math.min(deficit, heal.hp)
                    local target   = value.fought(snap, member, seen)
                    local fuel     = target ~= nil and value.convertible(member, target) or 0
                    local saved    = math.max(restored * fuel,
                        recovery.useful(heal.hp, deficit, member.self.max_hp or 0) *
                        value.vigor(member, query, worth, mana))
                    local leave    = leaving(snap, query, member, current, worth)
                    local averted  = rescue(snap, query, member, current, restored, heal, worth)
                    if leave ~= nil then
                        averted = averted + math.min(restored, leave.short) / leave.short * leave.cost
                    end
                    table.insert(found, {
                        urgent = dying,
                        dies = current / math.max(combat.pressure(snap, member, 1000), 1e-9),
                        oid = ally.oid,
                        slot = slots[heal.name],
                        key = heal.name,
                        cost = heal.mp,
                        averted = averted,
                        value = saved + averted - heal.mp * value.spend(snap, query, target),
                    })
                end
            end
        end
    end
    local deficit    = snap.self.max_hp - snap.self.hp
    local mine       = snap.self.hp / math.max(combat.pressure(snap, snap, 1000), 1e-9)
    local worthwhile = {}
    for _, heal in ipairs(combat.options(snap, {}).heal) do
        if slots[heal.name] ~= nil and heal.mp <= (snap.self.mp or 0) and heal.hp <= deficit and
            heal.hp > combat.pressure(snap, snap, combat.cast(snap)) then
            table.insert(worthwhile, heal)
        end
    end
    if #worthwhile > 0 and combat.pressure(snap, snap, 2 * combat.cast(snap)) >= snap.self.hp then
        local kept = {}
        for _, entry in ipairs(found) do
            if entry.dies == nil or entry.dies < mine then
                table.insert(kept, entry)
            end
        end
        if #kept < #found then
            local top = 0
            for _, entry in ipairs(found) do
                top = math.max(top, entry.value)
            end
            found = kept
            for _, heal in ipairs(worthwhile) do
                table.insert(found, {
                    self = true,
                    urgent = true,
                    slot = slots[heal.name],
                    key = heal.name,
                    cost = heal.mp,
                    value = top + heal.hp,
                })
            end
            return found
        end
    end
    if #found > 0 and deficit > 0 then
        local crew    = escort.present(snap, query)
        local helped  = (crew ~= nil and crew.heal or 0) * combat.cast(snap) / 1000
        local shared  = (snap.record.memory.battle or {}).oid
        local engaged = shared ~= nil and seen[shared] or nil
        local fuel    = engaged ~= nil and value.convertible(snap, engaged) or 0
        for _, heal in ipairs(combat.options(snap, {}).heal) do
            if slots[heal.name] ~= nil and heal.mp + reserve <= (snap.self.mp or 0) then
                local restored = math.max(0, math.min(deficit, heal.hp) - helped)
                local saved    = math.max(restored * fuel, restored * value.vigor(snap, query, worth, mana))
                table.insert(found, {
                    self = true,
                    slot = slots[heal.name],
                    key = heal.name,
                    cost = heal.mp,
                    value = saved + rescue(snap, query, snap, snap.self.hp, restored, heal, worth) -
                        heal.mp * value.spend(snap, query, engaged),
                })
            end
        end
    end
    return found
end

local function applied(snap, oid)
    local names  = combat.marks(snap, oid)
    local memory = snap.record.memory.battle
    if memory ~= nil and memory.oid == oid and memory.map == snap.world.map then
        for name, at in pairs(memory.applied or {}) do
            if combat.lasting(name, at, snap.now) then
                names[name] = true
            end
        end
    end
    for _, profile in ipairs(snap.party or {}) do
        local shared = profile.battle
        if type(shared) == "table" and shared.oid == oid and shared.map == snap.world.map then
            for name, at in pairs(shared.applied or {}) do
                if combat.lasting(name, at, snap.now) then
                    names[name] = true
                end
            end
        end
    end
    return names
end

local function mending(snap, query, mob, worth)
    local mana  = value.spend(snap, query, mob)
    local price = nil
    for _, heal in ipairs(combat.options(snap, {}).heal) do
        if heal.others and heal.hp > 0 then
            local each = ((heal.mp or 0) * mana + combat.cast(snap) / 1000 * worth.value) / heal.hp
            price      = price == nil and each or math.min(price, each)
        end
    end
    if price == nil then
        local regen = recovery.rates(snap).hp
        price       = regen > 0 and worth.value / regen or 0
    end
    return price
end

-- Holding a mob off a crewmate, or off me, is worth the death it spares within the window a heal is weighed in
-- (rescue) and the healing its blows would have needed while it is held. Weighed only by the time it takes off the
-- fight, 마비 was worth nothing: a mage beside four skeletons never held one off its warrior, which fell under a quarter
-- of its hp 152 times in 11 minutes (online WA+MA 152615).
local function guard(snap, query, member, mob, current, chance, held, before, worth)
    if held <= 0 or mob.x == nil or combat.hits(mob) == false or (mob.speed or 0) <= 0 or
        math.abs(mob.x - member.world.x) + math.abs(mob.y - member.world.y) ~= 1 then
        return 0
    end
    local window  = 2 * combat.cast(snap)
    local blows   = combat.pressure(snap, member, window)
    local _, high = combat.spread(member, mob)
    local rest    = blows - high * window / mob.speed
    local spared  = blows > 0 and math.exp(-current / blows) - (rest > 0 and math.exp(-current / rest) or 0) or 0
    local kept    = combat.blow(member, mob) * math.min(held, before.time) / mob.speed
    local leave   = leaving(snap, query, member, current, worth)
    local stays   = 0
    if leave ~= nil and leave.drains[tostring(mob.oid)] ~= nil then
        local spared_hp = leave.drains[tostring(mob.oid)] * math.min(held / 1000, leave.clear)
        stays           = math.min(spared_hp, leave.short) / leave.short * leave.cost
    end
    return chance * math.max(0, spared) * party.death(snap, query, member, worth) +
        kept * mending(snap, query, mob, worth) +
        stays
end

-- A debuff of a group only one of which a mob bears (저주, 혼마술) is worth what it adds over the strongest one a
-- crewmate here can cast: cast first, the weaker one barred the stronger, and a mage cursed ten mobs its poet was
-- casting 혼마술 at (online WA+PO+MA 150824).
local function exclusive(snap, query, name)
    local crew  = escort.present(snap, query)
    local found = {}
    for _, group in ipairs(data.debuffs or {}) do
        local inside = false
        for _, member in ipairs(group) do
            inside = inside or member == name
        end
        for _, member in ipairs(inside and group or {}) do
            if member ~= name and crew ~= nil and crew.assist[member] then
                found[member] = true
            end
        end
    end
    return found
end

local function debuffs(snap, query, slots, worth)
    local found   = {}
    local seen    = value.targets(snap)
    local helper  = escort.crew({ snap }, query)
    local guarded = { snap }
    local engaged = {}
    for _, member in ipairs(party.members(snap)) do
        table.insert(guarded, member)
    end
    for _, member in ipairs(guarded) do
        local own     = member == snap
        local shared  = own == false and member.battle or nil
        local list    = {}
        local chosen  = type(shared) == "table" and shared.map == snap.world.map and seen[shared.oid] or nil
        local ally    = own == false and query.ally(member.self.name) or nil
        local current = own and snap.self.hp or standing(member, ally)
        if chosen ~= nil then
            table.insert(list, chosen)
        end
        if member.world.map == snap.world.map and member.world.x ~= nil then
            for _, near in ipairs(snap.world.mobs or {}) do
                if near ~= chosen and near.x ~= nil and combat.hits(near) and
                    math.abs(near.x - member.world.x) + math.abs(near.y - member.world.y) == 1 then
                    table.insert(list, near)
                end
            end
        end
        for _, mob in ipairs(member.self.ghost == false and list or {}) do
            local names   = applied(snap, mob.oid)
            local context = escort.context(member, query, mob, helper, nil, 0)
            local before  = combat.outcome(member, query, copy(mob, { applied = names }), nil, context.aid)
            for name, slot in pairs(slots) do
                local spell = magic.spells[name]
                local cost  = spell ~= nil and spell.mp or nil
                if spell ~= nil and spell.role == "DEBUFF" and names[name] == nil and type(cost or 0) == "number" and
                    (cost or 0) <= recovery.free(snap) and (own == false or spell.paralysis or spell.blind) then
                    local rivals = exclusive(snap, query, name)
                    local base   = next(rivals) ~= nil and copy(names, rivals) or names
                    local from   = base == names and before or
                        combat.outcome(member, query, copy(mob, { applied = base }), nil, context.aid)
                    local after  = combat.outcome(member, query, copy(mob, { applied = copy(base, { [name] = true }) }),
                        nil, context.aid)
                    local saved  = own == false and from.time < math.huge and
                        math.max(0, from.time - after.time) / math.max(from.time, 1) or 0
                    local held   = combat.debuffs(member, copy(mob, { applied = { [name] = true } })).disable
                    local chance = held / math.max((spell.buff_time or 0) * 1000, 1)
                    local kept   = (spell.paralysis or spell.blind) and next(rivals) == nil and
                        guard(snap, query, member, mob, current, chance, held, before, worth) or 0
                    if saved > 0 or kept > 0 then
                        table.insert(found, {
                            oid = mob.oid,
                            slot = slot,
                            key = name,
                            debuff = name,
                            cost = cost or 0,
                            value = saved * combat.gain(mob, member) * (mob.ratio or 1) + kept -
                                (cost or 0) * value.spend(snap, query, mob),
                        })
                    end
                end
            end
            engaged[mob.oid] = true
        end
    end
    -- A mob in sight that nobody fights yet takes the ticks of a poison until it is reached, so poisoned first it is
    -- that much of its kill done meanwhile, short of the hp the poison never takes (dot_floor). Poisoned only once it
    -- was fought, five skeletons in sight were fought one after another at full hp (user 11-14, online WA+MA 152615).
    -- A mob that would not come by itself is drawn by the ticks, and its blows on me while poisoned are the cost. Only
    -- prey the crew hunts here and may fight where it stands is poisoned (field.fightable): poisoned beyond that, a mob
    -- at 1% six tiles off that nobody was allowed to strike was poisoned again each time the ticks ran out, and the
    -- crew waited on it for five minutes (online 004028, 00:43:36-00:48:30).
    local fought = snap.record.memory.battle
    local plan   = party.plan(snap, query)
    local hunted = {}
    for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
        for _, name in ipairs(floor.map == snap.world.map and floor.targets or {}) do
            hunted[name] = true
        end
    end
    local field = require("brain.tasks.field")
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.oid ~= nil and engaged[mob.oid] == nil and mob.x ~= nil and combat.hits(mob) and (mob.speed or 0) > 0 and
            (fought == nil or fought.oid ~= mob.oid) and math.abs(mob.x - snap.world.x) <= data.sight.x and
            math.abs(mob.y - snap.world.y) <= data.sight.y and hunted[mob.name] ~= nil and
            field.fightable(snap, query, mob) then
            local names     = applied(snap, mob.oid)
            local remaining = (mob.hp or 0) * (mob.ratio or 1)
            for name, slot in pairs(slots) do
                local spell = magic.spells[name]
                local cost  = spell ~= nil and spell.mp or nil
                if spell ~= nil and spell.role == "DEBUFF" and (spell.dot or 0) > 0 and names[name] == nil and
                    type(cost or 0) == "number" and (cost or 0) <= recovery.free(snap) and remaining > 0 then
                    local weak     = combat.debuffs(snap, copy(mob, { applied = { [name] = true } }))
                    local dealt    = weak.dot * (mob.hp or 0) * weak.dot_time / ((magic.buff_tick or 1) * 1000)
                    local done     = math.max(0, math.min(dealt, remaining - (spell.dot_floor or 0))) / remaining
                    local drawn    = mob.attack ~= "AGGRESSIVE" and
                        combat.blow(snap, mob) * weak.dot_time / mob.speed * mending(snap, query, mob, worth) or 0
                    local worth_of = done * combat.gain(mob, snap) - drawn - (cost or 0) * value.spend(snap, query, mob)
                    if done > 0 then
                        table.insert(found, {
                            oid = mob.oid,
                            slot = slot,
                            key = name,
                            debuff = name,
                            cost = cost or 0,
                            value = worth_of,
                        })
                    end
                end
            end
        end
    end
    return found
end

local function pending(snap, member, name, spell)
    local at = (snap.record.memory.warded or {})[member.self.name .. "|" .. name]
    return at ~= nil and snap.now - at < (spell.buff_time or 0) * 1000 and (member.now or 0) <= at
end

function M.ward(snap, query, target, name, worth, current)
    local spell  = magic.spells[name]
    local mana   = value.mana(snap, query)
    local plan   = party.plan(snap, query)
    local pain   = plan ~= nil and plan.pain or 0
    local derate = ((spell.boost or {}).damage_derate or 0) / 1000
    local share  = derate / (1 + derate)
    local armor  = (spell.boost or {}).buff_phydef or 0
    if armor > 0 then
        local defense = target.self.phydef or 0
        share = 1 - combat.mitigate(100, defense - armor, 0) / math.max(combat.mitigate(100, defense, 0), 1)
    end
    local saved = math.max(pain, 1) * (spell.buff_time or 0) * share
    local cost  = (spell.mp or 0) * mana
    local blows = combat.pressure(snap, target, 2 * combat.cast(snap))
    local hp    = math.max(current or target.self.hp or 0, 1)
    local kept  = math.exp(-blows / hp)
    return math.max(saved * value.vigor(target, query, worth, mana), share * party.death(snap, query, target, worth)) *
        kept -
        cost
end

-- A ward is cast on whoever it spares, its caster as well (spell.buff_cast takes any life): kept to the crew, a poet
-- cast 보호 on its warrior and took the blows of the skeletons held beside it at full weight, 300-400 each, until its
-- mana ran out and it fled (offline WA+PO 세작 seed 0, 690-724s, run 042648).
local function wards(snap, query, slots, worth)
    local found   = {}
    local targets = { { member = snap, hp = snap.self.hp } }
    for _, member in ipairs(party.members(snap)) do
        local ally = query.ally(member.self.name)
        if ally ~= nil and member.self.ghost == false then
            table.insert(targets, { member = member, oid = ally.oid, hp = standing(member, ally) })
        end
    end
    for _, target in ipairs(targets) do
        local member = target.member
        for name, slot in pairs(slots) do
            local spell = magic.spells[name]
            if combat.warding(name) and combat.warded(member.buffs, name) == false and
                (spell.mp or 0) <= recovery.free(snap) and
                pending(snap, member, name, spell) == false then
                table.insert(found, {
                    oid = target.oid,
                    self = target.oid == nil,
                    slot = slot,
                    key = name,
                    ward = member.self.name .. "|" .. name,
                    cost = spell.mp or 0,
                    value = M.ward(snap, query, member, name, worth, target.hp),
                })
            end
        end
    end
    return found
end

local function listed(snap, query)
    if plans[snap] ~= nil then
        return plans[snap]
    end
    local all = {}
    if snap.self.ghost == false then
        local slots   = {}
        local grouped = #party.members(snap) > 0
        for _, learned in ipairs(snap.spells or {}) do
            slots[learned.name] = learned.slot
        end
        local worth = value.baseline(snap, query)
        for group, candidates in pairs({ heal = grouped and heals(snap, query, slots, worth) or {},
            debuff = debuffs(snap, query, slots, worth), ward = grouped and wards(snap, query, slots, worth) or {} }) do
            for _, candidate in ipairs(candidates) do
                candidate.group = group
                table.insert(all, candidate)
            end
        end
    end
    plans[snap] = all
    return all
end

local function best_of(snap, query, accept)
    local best = nil
    for _, candidate in ipairs(listed(snap, query)) do
        if accept(candidate) and (best == nil or candidate.value > best.value) then
            best = candidate
        end
    end
    return best
end

local function action_of(snap, found)
    if found.walk ~= nil then
        return {
            kind = ACTION.MOVE,
            dest = { map = snap.world.map, x = found.walk.x, y = found.walk.y, range = math.min(data.sight.x, data.sight.y) },
        }
    end
    if found.self then
        return { kind = ACTION.CAST, slot = found.slot, key = found.key, ward = found.ward }
    end
    return {
        kind     = ACTION.ATTACK,
        oid      = found.oid,
        key      = found.key,
        ward     = found.ward,
        debuff   = found.debuff,
        maneuver = { kind = MANEUVER.CAST, slot = found.slot, self = false, cost = found.cost },
    }
end

local function urgent(candidate)
    return candidate.urgent == true
end

local function gainful(candidate)
    return candidate.value > 0
end

function M.urgent(snap, query)
    local found = best_of(snap, query, urgent)
    return found ~= nil and action_of(snap, found) or nil
end

-- The part of a heal that keeps a crewmate alive: the death it spares within the casts a heal is weighed in and the
-- leave its shortfall would force (rescue, leaving), without what the healed hp makes in strikes. My own step of a plan
-- gives way to a heal for that part alone (trees.main; user 11-15: step aside first unless a crewmate or I could die):
-- weighed whole, poisoning kept a mage off its step for 38s (online 20261115-120942), and with only the heals that save
-- a death within one cast, a poet stepped on beside its warrior at a tenth of its hp under two skeletons (brain unit test).
local function saving(snap, query)
    local found = nil
    for _, candidate in ipairs(listed(snap, query)) do
        if (candidate.averted or 0) > 0 and (found == nil or candidate.averted > found.averted) then
            found = candidate
        end
    end
    return found
end

function M.save(snap, query)
    local found = saving(snap, query)
    return found ~= nil and action_of(snap, found) or nil
end

function M.saved(snap, query)
    local found = saving(snap, query)
    return found ~= nil and found.averted or nil
end

function M.offer(group)
    return function(snap, query)
        local found = best_of(snap, query, function(candidate)
            return candidate.group == group and gainful(candidate)
        end)
        return found ~= nil and action_of(snap, found) or nil
    end
end

function M.worth(group)
    return function(snap, query)
        local found = best_of(snap, query, function(candidate)
            return group == "urgent" and urgent(candidate) or (candidate.group == group and gainful(candidate))
        end)
        return found ~= nil and found.value or nil
    end
end

function M.wanted(snap, query)
    return best_of(snap, query, gainful) ~= nil
end

function M.value(snap, query)
    return best_of(snap, query, gainful).value
end

function M.fueling(snap, query, mob)
    return value.fuel(snap, snap, party.members(snap), mob)
end

-- Out of a fight a cast on a crewmate is the strike at my prey I do not make in that cast, worth its share of the
-- kill net of its mana and the aggro it holds (outweighs). Ordered before the hunt and weighed alone, every heal
-- worth more than nothing came first: a rogue with no mana kept back cast 동해의기원 4,890 times and its trio
-- killed 760 -> 120 (offline profile trio, run 01:48). Under blows the strike back is weighed beside the heal
-- already (fight weigh), so this counts out of a fight only.
local forgone = setmetatable({}, { __mode = "k" })

function M.forgone(snap, query)
    if forgone[snap] == nil then
        local plan  = party.plan(snap, query)
        local names = nil
        for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
            if floor.map == snap.world.map then
                names = floor.targets
            end
        end
        local found = names ~= nil and require("brain.tasks.field").target(snap, query, names) or nil
        forgone[snap] = found ~= nil and math.max(0, battle.marginal(snap, found) -
            (found.choice.mp or 0) * M.fueling(snap, query, found.mob) -
            battle.holding(snap, query, found.mob, found.choice.damage)) or 0
    end
    return forgone[snap]
end

function M.spared(group)
    local worth = M.worth(group)
    return function(snap, query)
        local found = worth(snap, query)
        return found ~= nil and found - M.forgone(snap, query) or nil
    end
end

-- A strike is worth its share of the kill less what its mana would heal into a crewmate that turns hp into damage,
-- wherever I strike from: freeing a walled crewmate, clearing my own way, hunting or saving a crewmate. Only the
-- rescue went unpriced, and a poet sent to save a rogue cast 화염주 five tiles from its 건곤대나이 warrior at 1644 hp
-- (offline profile trio rooted seed 2, 516.8s, 7 of 25 rescue 화염주 beside such a warrior, run 21:55).
function M.outweighs(snap, query, found)
    return battle.marginal(snap, found) >= (found.choice.mp or 0) * M.fueling(snap, query, found.mob) +
        battle.holding(snap, query, found.mob, found.choice.damage)
end

return M
