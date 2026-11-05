local data     = require("brain.data.combat")
local items    = require("brain.data.items")
local magic    = require("brain.data.spells")
local bag      = require("brain.policies.bag")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local ground   = require("brain.policies.ground")
local recovery = require("brain.policies.recovery")
local survival = require("brain.policies.survival")
local value    = require("brain.policies.value")
local trace    = require("brain.trace")

local M = {}

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

local function slots(snap)
    local found = {}
    for _, learned in ipairs(snap.spells or {}) do
        found[learned.name] = learned.slot
    end
    return found
end

local function restore(effect, maximum)
    if effect == nil then
        return 0
    elseif effect.full then
        return maximum
    end
    return (effect.ratio or 0) * maximum + math.max(0, effect.amount or 0)
end

local function occupant(snap, x, y, except)
    for _, other in ipairs(snap.world.mobs or {}) do
        if other.x == x and other.y == y and other.oid ~= except then
            return other
        end
    end
    return nil
end

local function around(snap, except)
    local found = {}
    for _, other in ipairs(snap.world.mobs or {}) do
        if other.x ~= nil and other.oid ~= except and
            math.abs(other.x - snap.world.x) + math.abs(other.y - snap.world.y) == 1 then
            table.insert(found, other)
        end
    end
    return found
end

local function splashed(snap, mob, damage)
    local found = {}
    if mob.x == nil or damage <= 0 then
        return found
    end
    local dx     = mob.x > snap.world.x and 1 or mob.x < snap.world.x and -1 or 0
    local dy     = mob.y > snap.world.y and 1 or mob.y < snap.world.y and -1 or 0
    local active = {}
    for _, name in ipairs(snap.buffs or {}) do
        active[name] = true
    end
    for name, splash in pairs(magic.splashes or {}) do
        if active[name] then
            local tiles = splash.sides == 1 and { { mob.x + dx, mob.y + dy } } or
                { { mob.x + dy, mob.y + dx }, { mob.x - dy, mob.y - dx } }
            for _, tile in ipairs(tiles) do
                local other = occupant(snap, tile[1], tile[2], mob.oid)
                if other ~= nil then
                    table.insert(found, { mob = other, damage = damage * splash.rate })
                end
            end
        end
    end
    return found
end

local bursts = setmetatable({}, { __mode = "k" })

function M.burst(snap)
    local known = bursts[snap]
    if known ~= nil then
        return known
    end
    local attackers = {}
    for _, other in ipairs(snap.world.mobs or {}) do
        if other.x ~= nil and combat.hits(other) and
            math.abs(other.x - snap.world.x) + math.abs(other.y - snap.world.y) == 1 then
            table.insert(attackers, { mob = other, count = 1 })
        end
    end
    known = #attackers > 0 and combat.spike(snap, attackers) or 0
    bursts[snap] = known
    return known
end

local reaches = setmetatable({}, { __mode = "k" })

function M.reachable(snap)
    if reaches[snap] ~= nil then
        return reaches[snap]
    end
    local worst = 0
    for _, other in ipairs(snap.world.mobs or {}) do
        if combat.hits(other) then
            worst = math.max(worst, combat.spike(snap, { { mob = other, count = 1 } }))
        end
    end
    reaches[snap] = worst
    return worst
end

local function bears(snap, facts, mob, choice)
    local mobs = {}
    for _, seen in ipairs(snap.world.mobs or {}) do
        if seen.oid ~= mob.oid then
            table.insert(mobs, seen)
        elseif (seen.hp or 0) * (seen.ratio or 1) > choice.damage then
            table.insert(mobs,
                copy(seen, { ratio = ((seen.hp or 0) * (seen.ratio or 1) - choice.damage) / math.max(seen.hp or 1, 1) }))
        end
    end
    local after = copy(snap, {
        self = copy(snap.self, { hp = snap.self.hp - choice.hp, mp = math.max(0, (snap.self.mp or 0) - choice.mp) }),
        world = copy(snap.world, { mobs = mobs }),
    })
    return danger.fatal(after, facts) == nil
end

local function mends(snap, facts, mp, hp)
    local crew   = escort.present(snap, facts)
    local cohort = escort.gathering(snap, facts)
    if cohort ~= nil and (crew == nil or #cohort.members > #crew.members) then
        return false
    end
    local missing = snap.self.max_hp - snap.self.hp
    local heal    = recovery.heal(snap)
    if crew ~= nil and crew.heal > 0 then
        return missing <= crew.heal * combat.cast(snap) / 1000
    end
    local stock = 0
    local dose  = 0
    for _, entry in ipairs(recovery.doses(snap, "hp")) do
        stock = stock + entry.amount * entry.count
        dose  = math.max(dose, entry.amount)
    end
    if heal ~= nil and heal.hp > 0 and heal.mp > 0 then
        stock = stock + heal.hp * mp // heal.mp
        dose  = math.max(dose, heal.hp)
    end
    return stock >= hp and missing <= dose
end

local function candidates(snap, facts, mob, state, reachable)
    local target    = combat.exposed(mob, combat.debuffs(snap, copy(mob, { applied = state.applied })))
    local options   = combat.options(snap, target)
    local known     = slots(snap)
    local cast      = combat.cast(snap) + combat.cooldown(snap)
    local swing     = data.swing
    local found     = {}
    local spendable = recovery.spendable(snap)
    local free      = recovery.free(snap)
    local add       = function(entry)
        local limit = entry.kind == "HEAL" and spendable or free
        if state.refused[entry.key] == nil and (entry.mp or 0) <= limit then
            table.insert(found, entry)
        end
    end
    table.sort(options.offense, function(left, right)
        return left.damage > right.damage
    end)
    local adjacent = mob.x ~= nil and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1
    local burst    = M.burst(snap)
    local opener   = combat.opener(snap, mob)
    local reserve  = math.max(recovery.spare(snap), opener ~= nil and opener.mp or 0)
    for _, choice in ipairs(options.offense) do
        local spell = magic.spells[choice.name]
        local area  = spell ~= nil and spell.role == "AREA" and spell.type == "NORMAL"
        if choice.damage > 0 and known[choice.name] ~= nil and choice.mp + reserve <= (snap.self.mp or 0) and
            (choice.hp >= 0 or snap.self.hp + choice.hp > burst) and ((area or choice.front) == false or adjacent) then
            local extra = {}
            if area then
                for _, other in ipairs(around(snap, mob.oid)) do
                    table.insert(extra, { mob = other, damage = choice.damage })
                end
            end
            add({
                key = choice.name,
                kind = "CAST",
                time = cast,
                damage = choice.damage,
                mp = choice.mp,
                hp = choice.hp,
                extra = extra,
                maneuver = { kind = MANEUVER.CAST, slot = known[choice.name], cost = choice.mp }
            })
        end
    end
    for _, choice in ipairs(options.strike) do
        if known[choice.name] ~= nil and choice.mp <= (snap.self.mp or 0) and adjacent and
            snap.self.hp - choice.hp > burst and mends(snap, facts, (snap.self.mp or 0) - choice.mp, choice.hp) and
            bears(snap, facts, mob, choice) then
            local spell = magic.spells[choice.name]
            add({
                key = choice.name,
                kind = "STRIKE",
                time = cast,
                damage = choice.damage,
                mp = choice.mp,
                spend = spell ~= nil and (spell.damage_mp or 0) > 0 and (recovery.opening(snap) or choice.mp) or nil,
                hp = -choice.hp,
                opener = true,
                maneuver = { kind = MANEUVER.CAST, slot = known[choice.name], cost = choice.mp }
            })
        end
    end
    for name, slot in pairs(known) do
        local spell = magic.spells[name]
        if spell ~= nil and spell.role == "DEBUFF" and state.applied[name] == nil and
            type(spell.mp or 0) == "number" and (spell.mp or 0) <= (snap.self.mp or 0) then
            add({
                key = name,
                kind = "DEBUFF",
                time = cast,
                mp = spell.mp or 0,
                debuff = name,
                maneuver = { kind = MANEUVER.CAST, slot = slot, cost = spell.mp or 0 }
            })
        end
    end
    local missing = snap.self.max_hp - snap.self.hp
    local mending = missing > 0 and recovery.mending(snap, missing) or nil
    local crew    = escort.present(snap, facts)
    local helped  = (crew ~= nil and crew.heal or 0) * cast / 1000
    for _, choice in ipairs(options.heal) do
        if mending ~= nil and known[choice.name] == mending.slot and choice.hp > helped then
            add({
                key = choice.name,
                kind = "HEAL",
                time = cast,
                mp = choice.mp,
                hp = choice.hp - helped,
                wasted = choice.mp * math.max(0, 1 - missing / math.min(choice.hp, snap.self.max_hp)),
                maneuver = { kind = MANEUVER.CAST, slot = known[choice.name], self = true }
            })
        end
    end
    local charge = recovery.spells(snap).charge
    for name, count in pairs(bag.usable(snap)) do
        local effect = count > 0 and items[name] or nil
        if effect ~= nil then
            local hp    = restore({ full = effect.hp_full, ratio = effect.hp_ratio, amount = effect.hp },
                snap.self.max_hp)
            local mp    = restore({ full = effect.mp_full, ratio = effect.mp_ratio, amount = effect.mp },
                snap.self.max_mp or 0)
            local model = facts.item(name)
            if hp <= 0 and mp > 0 and charge ~= nil and (snap.self.mp or 0) >= charge.mp then
                mp = 0
            end
            if hp > 0 or mp > 0 then
                add({
                    key = name,
                    kind = "USE",
                    time = swing,
                    hp = hp,
                    mp = -mp,
                    price = model ~= nil and model.price or 0,
                    maneuver = { kind = MANEUVER.USE, item = name }
                })
            end
        end
    end
    local charging = combat.buffs(snap).charge
    if charge ~= nil and charging ~= nil and (snap.self.mp or 0) >= charge.mp and (snap.self.mp or 0) < (snap.self.max_mp or 0) then
        local change = recovery.charged(snap) - snap.self.hp
        local refill = (snap.self.max_mp or 0) - (snap.self.mp or 0) + charge.mp
        add({
            key = "CHARGE",
            kind = "CHARGE",
            time = cast,
            mp = charge.mp - charging.chance * refill,
            hp = charging.chance * change,
            dip = (1 - charging.chance) * math.max(0, -change),
            maneuver = { kind = MANEUVER.CAST, slot = charge.slot, self = true }
        })
    end
    local strike = combat.strike(snap, facts, target)
    local melee  = strike / swing * 1000
    for _, entry in ipairs(found) do
        if entry.kind == "STRIKE" then
            entry.melee = melee
        end
    end
    local cloak = 1
    for name, slot in pairs(known) do
        local factor = (magic.multipliers or {})[name]
        local spell  = magic.spells[name]
        if factor ~= nil and snap.self.cloaked == true then
            cloak = math.max(cloak, factor)
        elseif factor ~= nil and spell ~= nil and (spell.mp or 0) <= (snap.self.mp or 0) then
            add({
                key = name,
                kind = "CAST",
                time = cast,
                damage = strike * (factor - 1),
                mp = spell.mp or 0,
                maneuver = { kind = MANEUVER.CAST, slot = slot, self = true, cost = spell.mp or 0 }
            })
        end
    end
    for name, slot in pairs(known) do
        local spell = magic.spells[name]
        if cloak > 1 and spell ~= nil and spell.role == "AMBUSH" and (spell.mp or 0) <= (snap.self.mp or 0) and adjacent then
            add({
                key = name,
                kind = "CAST",
                time = math.max(cast, swing),
                damage = strike * cloak * magic.back_attack,
                mp = spell.mp or 0,
                moves = true,
                maneuver = { kind = MANEUVER.CAST, slot = slot, cost = spell.mp or 0 }
            })
        end
    end
    if reachable ~= false then
        add({
            key = "SWING",
            kind = "SWING",
            time = swing + (adjacent and combat.backswing(snap) or 0),
            damage = strike * cloak,
            extra = splashed(snap, mob, strike),
            approach = true,
            maneuver = { kind = MANEUVER.SWING }
        })
    end
    return found
end

local function delay(snap, mob, entry, regen, taken)
    local spent = -math.min(entry.hp or 0, 0)
    local heal  = regen - taken * 1000 / entry.time
    local top   = snap.self.max_hp
    if spent <= 0 or heal <= 0 or snap.self.hp >= top then
        return 0
    end
    local cast = entry.time / 1000
    local rate = function(damage, debt)
        local mend = debt / heal
        return (damage + entry.melee * mend) / (cast + mend)
    end
    local now  = rate(entry.damage, spent)
    if now <= 0 then
        return 0
    end
    local full = rate(entry.damage + top - snap.self.hp, spent * top / snap.self.hp)
    local wait = (top - snap.self.hp) / heal
    local left = (mob.hp or 0) * (mob.ratio or 1)
    return math.max(0, (full - now) * left / now - (now - entry.melee) * wait) / now
end

local function estimate(snap, facts, mob, entry, worth, loss, mana)
    local hits   = combat.hits(mob) and mob.speed ~= nil and mob.speed > 0
    local taken  = (hits and entry.time / mob.speed * combat.blow(snap, mob) or 0) +
        survival.contact(snap, mob, entry.time).pressure * entry.time
    local now    = combat.lethal(snap, mob, entry.time, snap.self.hp)
    local left   = (mob.hp or 0) * (mob.ratio or 1) - (entry.damage or 0)
    local hp     = math.min(snap.self.max_hp, math.max(0, snap.self.hp - taken) + math.max(0, entry.hp or 0)) +
        math.min(0, entry.hp or 0)
    local mp     = math.min(snap.self.max_mp or 0, math.max(0, (snap.self.mp or 0) - (entry.mp or 0)))
    local rest   = { time = 0, p_die = 0 }
    local ending = hp
    local refill = 0
    if left > 0 and hp > 0 then
        local plenty = combat.buffs(snap).charge
        local ahead  = recovery.heal(snap)
        local fresh  = { hp = hp, mp = mp }
        if entry.kind == "HEAL" and plenty ~= nil and ahead ~= nil and mp >= plenty.mp and mp < ahead.mp + plenty.mp and
            hp > plenty.cost + 2 * M.burst(snap) then
            fresh  = {
                hp = hp - plenty.chance * plenty.cost,
                mp = mp - plenty.mp + plenty.chance * ((snap.self.max_mp or 0) - mp + plenty.mp)
            }
            refill = combat.cast(snap)
        end
        local after   = copy(snap, { self = copy(snap.self, fresh) })
        local applied = {}
        for name in pairs(entry.applied or {}) do
            applied[name] = true
        end
        if entry.debuff ~= nil then
            applied[entry.debuff] = true
        end
        local prey    = copy(mob, { ratio = left / math.max(mob.hp or 1, 1), applied = applied })
        local context = escort.context(after, facts, prey, escort.present(snap, facts), nil, recovery.rates(after).hp)
        rest          = survival.fight(after, facts, prey, context)
        context.crowd = survival.contact(after, prey, rest.time)
        rest          = survival.fight(after, facts, prey, context)
        ending        = rest.ending
    elseif hp <= 0 then
        rest = { time = 0, p_die = 1 }
    end
    local p_die = 1 - (1 - now) * (1 - rest.p_die)
    local blows = M.burst(snap)
    if blows > 0 and left > 0 then
        local helper = escort.present(snap, facts)
        local cover  = hp + (helper ~= nil and helper.heal or 0) * combat.cast(snap) / 1000
        p_die        = 1 - (1 - p_die) * (1 - math.max(0, math.min(1, (2 * blows - cover) / blows)))
    end
    local mend = recovery.heal(snap)
    if refill == 0 and entry.kind ~= "CHARGE" and combat.buffs(snap).charge ~= nil and mend ~= nil and hp > 0 and
        mp < mend.mp + combat.buffs(snap).charge.mp then
        local exposed = 1 - math.min(1, M.burst(snap) / hp)
        p_die = 1 - (1 - p_die) * (mp < combat.buffs(snap).charge.mp and exposed * exposed or exposed)
    end
    local time     = (entry.time + refill + rest.time) / 1000
    local ready    = ground.need(snap, facts)
    local charging = combat.buffs(snap).charge
    if charging ~= nil then
        ready = math.max(ready, charging.cost + 2 * M.burst(snap))
    end
    local short = math.max(0, ready - math.max(ending, 0))
    local crew  = escort.present(snap, facts)
    local regen = recovery.rates(snap).hp + data.natural.hp * math.max(snap.self.max_hp, 1) +
        (crew ~= nil and crew.heal or 0)
    if short > 0 then
        time = time + short / regen
    end
    if entry.melee ~= nil then
        time = time + delay(snap, mob, entry, regen, taken)
    elseif (entry.hp or 0) < 0 and regen > 0 then
        time = time - entry.hp / regen
    end
    if (entry.wasted or 0) > 0 then
        time = time + entry.wasted / (recovery.rates(snap).mp + data.natural.mp * math.max(snap.self.max_mp or 0, 1))
    end
    local bonus = 0
    for _, hit in ipairs(entry.extra or {}) do
        local remaining = (hit.mob.hp or 0) * (hit.mob.ratio or 1)
        if remaining > 0 then
            bonus = bonus + combat.gain(hit.mob) * math.min(1, hit.damage / remaining)
        end
    end
    local risk = (1 - p_die) * (combat.gain(mob) + bonus) - p_die * loss
    local plan = { time = time * 1000, p_die = p_die, ending = ending, rest = rest.time, extra = #(entry.extra or {}) }
    return
        risk - time * worth.value - (entry.price or 0) * worth.exchange -
        ((entry.spend or entry.mp or 0) + (rest.drained or 0)) * mana,
        ending > 0 and hp - (entry.dip or 0) > (entry.kind == "CHARGE" and 2 or 1) * M.burst(snap) and
        hp - (entry.dip or 0) > combat.spike(snap, { { mob = mob, count = 1 } }), ending,
        plan
end

local function lasting(snap, name, at)
    if combat.lasting(name, at, snap.now) == false then
        return false
    end
    local spell = magic.spells[name]
    return spell == nil or (spell.paralysis ~= true and spell.blind ~= true) or snap.self.struck == nil or
        snap.self.struck > snap.now - at
end

local function remembered(snap, mob)
    local state  = { opened = false, refused = {}, applied = {} }
    local memory = snap.record.memory.battle
    if memory ~= nil and memory.oid == mob.oid then
        state.opened = memory.opened == true
        if memory.refused ~= nil then
            state.refused[memory.refused] = true
        end
        for name, at in pairs(memory.applied or {}) do
            if lasting(snap, name, at) then
                state.applied[name] = true
            end
        end
    end
    for _, profile in ipairs(snap.party or {}) do
        local shared = profile.battle
        if type(shared) == "table" and shared.oid == mob.oid and shared.map == snap.world.map then
            for name, at in pairs(shared.applied or {}) do
                if lasting(snap, name, at) then
                    state.applied[name] = true
                end
            end
        end
    end
    return state
end

local evaluate = nil
local endings  = setmetatable({}, { __mode = "k" })
local targets  = {}
local SAMPLE   = 5

local chosen = setmetatable({}, { __mode = "k" })

local function kindred(snap, mob, flag)
    if mob.x == nil or mob.id == nil then
        return nil
    end
    local distance = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
    local memory   = snap.record.memory.battle
    if distance <= 1 or (memory ~= nil and memory.oid == mob.oid) then
        return nil
    end
    for _, profile in ipairs(snap.party or {}) do
        if type(profile.battle) == "table" and profile.battle.oid == mob.oid then
            return nil
        end
    end
    for _, name in ipairs(snap.buffs or {}) do
        if (magic.splashes or {})[name] ~= nil then
            return nil
        end
    end
    return table.concat({ mob.id, mob.ratio or 1, distance, tostring(flag) }, "|")
end

function M.choose(snap, facts, mob, reachable)
    local bysnap = chosen[snap]
    if bysnap == nil then
        bysnap = {}
        chosen[snap] = bysnap
    end
    local byreach = bysnap[mob]
    if byreach == nil then
        byreach = {}
        bysnap[mob] = byreach
    end
    local flag = reachable == nil and "any" or reachable
    if byreach[flag] == nil then
        local key  = kindred(snap, mob, flag)
        local same = key ~= nil and bysnap[key] or nil
        if same == nil then
            same = {
                choice = evaluate(snap, facts, mob, reachable) or false,
                ending = endings[snap] ~= nil and endings[snap][mob.oid or mob.name] or nil
            }
            if key ~= nil then
                bysnap[key] = same
            end
        else
            endings[snap] = endings[snap] or {}
            endings[snap][mob.oid or mob.name] = same.ending
        end
        byreach[flag] = same.choice
    end
    return byreach[flag] or nil
end

function evaluate(snap, facts, mob, reachable)
    local baseline = value.baseline(snap, facts)
    local loss     = danger.loss(snap, facts, baseline)
    local mana     = value.refill(snap, facts, baseline)
    local best     = nil
    local values   = {}
    local ending   = -math.huge
    local last     = nil
    local state    = remembered(snap, mob)
    for _, entry in ipairs(candidates(snap, facts, mob, state, reachable)) do
        entry.applied = state.applied
        local left
        entry.value, entry.survives, left, entry.plan = estimate(snap, facts, mob, entry, baseline, loss, mana)
        if last == nil or entry.value > last.value then
            last = entry
        end
        ending = math.max(ending, left)
        if data.trace then
            table.insert(values, string.format("%s:%s=%.1f%s", entry.kind, entry.key, entry.value,
                entry.survives and "" or "!"))
        end
        if entry.survives and (best == nil or entry.value > best.value or
                (entry.value == best.value and ((entry.damage or 0) > (best.damage or 0) or
                    ((entry.damage or 0) == (best.damage or 0) and entry.key < best.key)))) then
            best = entry
        end
    end
    endings[snap] = endings[snap] or {}
    endings[snap][mob.oid or mob.name] = ending
    best = best or last
    if data.trace then
        log("debug", string.format("[brain:battle] bot=%s hp=%d mp=%d mob=%s %s picks %s", snap.self.name or "",
            snap.self.hp, snap.self.mp or 0, mob.name or "", table.concat(values, " "),
            best ~= nil and best.key or "none"))
    end
    return best
end

function M.need(snap, facts)
    local nearest = nil
    local closest = math.huge
    for _, mob in ipairs(snap.world.mobs) do
        if mob.x ~= nil and mob.attack == "AGGRESSIVE" and combat.hits(mob) then
            local distance = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
            if distance < closest then
                nearest = mob
                closest = distance
            end
        end
    end
    if nearest == nil then
        return 0
    end
    local known = endings[snap] ~= nil and endings[snap][nearest.oid or nearest.name] or nil
    if known == nil then
        M.choose(snap, facts, nearest)
        known = endings[snap][nearest.oid or nearest.name]
    end
    return snap.self.hp - known + 1
end

function M.target(snap, facts, accept)
    local step  = snap.pace.step or data.walk
    local worth = value.baseline(snap, facts).value
    local found = nil
    local best  = nil
    local near  = {}
    local goals = {}
    for _, mob in ipairs(snap.world.mobs) do
        if mob.x ~= nil and M.blocked(snap, mob) == false and accept(mob) then
            local distance = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
            table.insert(near, { mob = mob, distance = distance })
            table.insert(goals, { x = mob.x, y = mob.y, range = 1 })
        end
    end
    local paths = #goals > 0 and facts.steps ~= nil and facts.steps(snap.world.map, snap.world.x, snap.world.y, goals) or
        {}
    for index, entry in ipairs(near) do
        local steps     = paths[index]
        entry.reachable = steps ~= false
        entry.steps     = type(steps) == "number" and steps or math.max(0, entry.distance - 1)
    end
    table.sort(near, function(left, right)
        return left.steps < right.steps
    end)
    for _, entry in ipairs(near) do
        local travel = entry.reachable and entry.steps * step / 1000 * worth or 0
        if best == nil or combat.gain(entry.mob) > best then
            local choice = M.choose(snap, facts, entry.mob, entry.reachable)
            local net    = choice ~= nil and choice.value - (choice.approach and travel or 0) or nil
            if net ~= nil and (best == nil or net > best) then
                found = { mob = entry.mob, choice = choice }
                best  = net
            end
        end
    end
    if found ~= nil then
        found.forecast = M.forecast(snap, facts, found.mob, found.choice)
        local owner    = snap.self.name or ""
        local last     = targets[owner]
        if last == nil or last.oid ~= found.mob.oid then
            local count    = (last ~= nil and last.count or 0) + 1
            targets[owner] = { oid = found.mob.oid, count = count }
            if count % SAMPLE == 1 then
                found.forecast.snapshot = M.capture(snap)
            end
        end
    end
    return found
end

function M.capture(snap)
    local memory = snap.record.memory or {}
    return {
        self = snap.self,
        buffs = snap.buffs,
        spells = snap.spells,
        gear = snap.gear,
        bag = snap.bag,
        world = { map = snap.world.map, x = snap.world.x, y = snap.world.y, mobs = snap.world.mobs, items = snap.world.items },
        record = { failures = {}, memory = { battle = memory.battle, perils = memory.perils } },
        pace = snap.pace,
        now = snap.now,
        persona = snap.persona,
        group = snap.group,
        party = snap.party,
    }
end

function M.forecast(snap, facts, mob, choice)
    local crew    = escort.present(snap, facts)
    local context = escort.context(snap, facts, mob, crew)
    local fight   = survival.fight(snap, facts, mob, context)
    local model   = combat.outcome(snap, facts, mob, nil, context.aid)
    local finite  = function(number)
        return number ~= nil and number < math.huge and number > -math.huge and number or -1
    end
    return {
        key = choice.key,
        kind = choice.kind,
        value = finite(choice.value),
        step = choice.plan,
        crew = crew ~= nil and #crew.members or 0,
        aid = finite(context.aid or 0),
        hp = snap.self.hp,
        mp = snap.self.mp or 0,
        ratio = mob.ratio or 1,
        fight = {
            time = finite(fight.time),
            taken = finite(fight.taken),
            healed = finite(fight.healed),
            ending = finite(fight.ending),
            p_die = finite(fight.p_die),
            spent = finite(fight.spent or 0)
        },
        model = {
            time = finite(model.time),
            casts = finite(model.casts),
            swings = finite(model.swings),
            taken = finite(model.taken),
            healed = finite(model.healed),
            drained = finite(model.drained)
        },
    }
end

function M.propose(found, extra)
    local action = {
        kind = ACTION.ATTACK,
        oid = found.mob.oid,
        maneuver = found.choice.maneuver,
        key = found.choice.key,
        opener = found.choice.opener == true,
        moves = found.choice.moves == true,
        debuff = found.choice.debuff,
        mob = found.mob.name,
        mob_id = found.mob.id,
        forecast = found.forecast,
    }
    for key, entry in pairs(extra or {}) do
        action[key] = entry
    end
    return action
end

local GOING = { STRUCK = true, DONE = true, REFUSED = true, APPROACHED = true }

function M.going(kind)
    return GOING[kind] == true
end

function M.blocked(snap, mob)
    local entry = (snap.record.memory.unreachable or {})[tostring(mob.oid)]
    return entry ~= nil and entry.map == snap.world.map and entry.x == mob.x and entry.y == mob.y
end

function M.settle(memory, blocked, oid, map, action, event, now, visible, moved)
    if action.moves and event.type == "DONE" and moved == false then
        event = { type = "REFUSED" }
    end
    if memory == nil or memory.oid ~= oid or memory.map ~= map then
        memory = { oid = oid, map = map, opened = false }
    end
    memory.opened = memory.opened or action.opener == true
    if action.debuff ~= nil and event.type == "DONE" then
        memory.applied                = memory.applied or {}
        memory.applied[action.debuff] = now
    end
    memory.refused = event.type == "REFUSED" and action.key or nil
    local kept     = {}
    local count    = 0
    for key, entry in pairs(blocked or {}) do
        local mob = visible[key]
        if mob ~= nil and entry.map == map and mob.x == entry.x and mob.y == entry.y then
            kept[key] = entry
            count     = count + 1
        end
    end
    if event.type == "UNREACHABLE" and visible[tostring(oid)] ~= nil then
        local mob           = visible[tostring(oid)]
        kept[tostring(oid)] = { map = map, x = mob.x, y = mob.y }
        count               = count + 1
    end
    return GOING[event.type] and memory or nil, count > 0 and kept or nil
end

function M.step(brain, bot, oid, action)
    local seen    = brain:observe(bot)
    local before  = bot:position()
    local event   = brain:engage(bot, oid, action.maneuver)
    local after   = bot:position()
    local visible = {}
    for _, mob in ipairs(brain:mobs(bot)) do
        visible[tostring(mob.oid)] = mob
    end
    trace.engage(brain, bot, oid, action, event.type)
    local memory, blocked = M.settle(brain:recall(bot, "battle"), brain:recall(bot, "unreachable"), oid, seen.map,
        action, event, brain:observe(bot).now, visible, before[1] ~= after[1] or before[2] ~= after[2])
    brain:remember(bot, "battle", memory)
    brain:remember(bot, "unreachable", blocked)
    return event
end

return M
