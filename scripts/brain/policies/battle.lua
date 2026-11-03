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

local function candidates(snap, facts, mob, state)
    local target  = combat.exposed(mob, combat.debuffs(snap, copy(mob, { applied = state.applied })))
    local options = combat.options(snap, target)
    local known   = slots(snap)
    local cast    = combat.cast(snap) + combat.cooldown(snap)
    local swing   = data.swing
    local found   = {}
    local add     = function(entry)
        if state.refused[entry.key] == nil then
            table.insert(found, entry)
        end
    end
    table.sort(options.offense, function(left, right)
        return left.damage > right.damage
    end)
    for _, choice in ipairs(options.offense) do
        if choice.damage > 0 and known[choice.name] ~= nil and choice.mp <= (snap.self.mp or 0) then
            add({
                key = choice.name,
                kind = "CAST",
                time = cast,
                damage = choice.damage,
                mp = choice.mp,
                maneuver = { kind = MANEUVER.CAST, slot = known[choice.name] }
            })
        end
    end
    for _, choice in ipairs(options.strike) do
        if state.opened == false and known[choice.name] ~= nil and choice.mp <= (snap.self.mp or 0) then
            add({
                key = choice.name,
                kind = "STRIKE",
                time = cast,
                damage = choice.damage,
                mp = choice.mp,
                hp = -choice.hp,
                opener = true,
                maneuver = { kind = MANEUVER.CAST, slot = known[choice.name] }
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
    for _, choice in ipairs(options.heal) do
        if known[choice.name] ~= nil and choice.mp <= (snap.self.mp or 0) and choice.hp > 0 then
            add({
                key = choice.name,
                kind = "HEAL",
                time = cast,
                mp = choice.mp,
                hp = choice.hp,
                maneuver = { kind = MANEUVER.CAST, slot = known[choice.name], self = true }
            })
        end
    end
    for name, count in pairs(bag.usable(snap)) do
        local effect = count > 0 and items[name] or nil
        if effect ~= nil then
            local hp    = restore({ full = effect.hp_full, ratio = effect.hp_ratio, amount = effect.hp },
                snap.self.max_hp)
            local mp    = restore({ full = effect.mp_full, ratio = effect.mp_ratio, amount = effect.mp },
                snap.self.max_mp or 0)
            local model = facts.item(name)
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
    local strike = combat.strike(snap, facts, target)
    local cloak  = 1
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
        if cloak > 1 and spell ~= nil and spell.role == "AMBUSH" and (spell.mp or 0) <= (snap.self.mp or 0) then
            add({
                key = name,
                kind = "CAST",
                time = cast + swing,
                damage = strike * cloak * magic.back_attack,
                mp = spell.mp or 0,
                maneuver = { kind = MANEUVER.CAST, slot = slot, cost = spell.mp or 0 }
            })
        end
    end
    add({
        key = "SWING",
        kind = "SWING",
        time = swing,
        damage = strike * cloak,
        maneuver = { kind = MANEUVER.SWING }
    })
    return found
end

local function estimate(snap, facts, mob, entry, worth, loss)
    local hits   = combat.hits(mob) and mob.speed ~= nil and mob.speed > 0
    local taken  = hits and entry.time / mob.speed * combat.blow(snap, mob) or 0
    local now    = combat.lethal(snap, mob, entry.time, snap.self.hp)
    local left   = (mob.hp or 0) * (mob.ratio or 1) - (entry.damage or 0)
    local hp     = math.min(snap.self.max_hp, math.max(0, snap.self.hp - taken) + math.max(0, entry.hp or 0)) +
        math.min(0, entry.hp or 0)
    local mp     = math.min(snap.self.max_mp or 0, math.max(0, (snap.self.mp or 0) - (entry.mp or 0)))
    local rest   = { time = 0, p_die = 0 }
    local ending = hp
    if left > 0 and hp > 0 then
        local after   = copy(snap, { self = copy(snap.self, { hp = hp, mp = mp }) })
        local applied = {}
        for name in pairs(entry.applied or {}) do
            applied[name] = true
        end
        if entry.debuff ~= nil then
            applied[entry.debuff] = true
        end
        local prey    = copy(mob, { ratio = left / math.max(mob.hp or 1, 1), applied = applied })
        local context = escort.context(after, facts, prey, escort.present(snap, facts))
        rest          = survival.fight(after, facts, prey, context)
        context.crowd = survival.contact(after, prey, rest.time)
        rest          = survival.fight(after, facts, prey, context)
        ending        = rest.ending
    elseif hp <= 0 then
        rest = { time = 0, p_die = 1 }
    end
    local p_die = 1 - (1 - now) * (1 - rest.p_die)
    local time  = (entry.time + rest.time) / 1000
    local short = math.max(0, ground.need(snap, facts) - math.max(ending, 0))
    if short > 0 then
        local crew  = escort.present(snap, facts)
        local regen = recovery.rates(snap).hp + data.natural.hp * math.max(snap.self.max_hp, 1) +
            (crew ~= nil and crew.heal or 0)
        time        = time + short / regen
    end
    local risk = (1 - p_die) * combat.gain(mob) - p_die * loss
    return risk - time * worth.value - (entry.price or 0) * worth.exchange, ending > 0, ending
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
            if combat.lasting(name, at, snap.now) then
                state.applied[name] = true
            end
        end
    end
    for _, profile in ipairs(snap.party or {}) do
        local shared = profile.battle
        if type(shared) == "table" and shared.oid == mob.oid and shared.map == snap.world.map then
            for name, at in pairs(shared.applied or {}) do
                if combat.lasting(name, at, snap.now) then
                    state.applied[name] = true
                end
            end
        end
    end
    return state
end

local endings = setmetatable({}, { __mode = "k" })

function M.choose(snap, facts, mob)
    local baseline = value.baseline(snap, facts)
    local loss     = danger.loss(snap, facts, baseline)
    local best     = nil
    local values   = {}
    local ending   = -math.huge
    local state    = remembered(snap, mob)
    for _, entry in ipairs(candidates(snap, facts, mob, state)) do
        entry.applied = state.applied
        local left
        entry.value, entry.survives, left = estimate(snap, facts, mob, entry, baseline, loss)
        ending = math.max(ending, left)
        table.insert(values, string.format("%s:%s=%.1f%s", entry.kind, entry.key, entry.value,
            entry.survives and "" or "!"))
        if entry.survives and (best == nil or entry.value > best.value) then
            best = entry
        end
    end
    endings[snap] = endings[snap] or {}
    endings[snap][mob.oid or mob.name] = ending
    log("debug", string.format("[brain:battle] bot=%s hp=%d mp=%d mob=%s %s picks %s", snap.self.name or "",
        snap.self.hp, snap.self.mp or 0, mob.name or "", table.concat(values, " "), best ~= nil and best.key or "none"))
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
    for _, mob in ipairs(snap.world.mobs) do
        if mob.x ~= nil and accept(mob) then
            local distance = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
            table.insert(near, { mob = mob, distance = distance })
        end
    end
    table.sort(near, function(left, right)
        return left.distance < right.distance
    end)
    for _, entry in ipairs(near) do
        local travel = entry.distance * step / 1000 * worth
        if best == nil or combat.gain(entry.mob) - travel > best then
            local choice = M.choose(snap, facts, entry.mob)
            if choice ~= nil and (best == nil or choice.value - travel > best) then
                found = { mob = entry.mob, choice = choice }
                best  = choice.value - travel
            end
        end
    end
    return found
end

function M.propose(found, extra)
    local action = {
        kind = ACTION.ATTACK,
        oid = found.mob.oid,
        maneuver = found.choice.maneuver,
        key = found.choice.key,
        opener = found.choice.opener == true,
        debuff = found.choice.debuff
    }
    for key, entry in pairs(extra or {}) do
        action[key] = entry
    end
    return action
end

function M.step(brain, bot, oid, action)
    local memory = brain:recall(bot, "battle")
    local seen   = brain:observe(bot)
    if memory == nil or memory.oid ~= oid or memory.map ~= seen.map then
        memory = { oid = oid, map = seen.map, opened = false }
    end
    local event   = brain:engage(bot, oid, action.maneuver)
    memory.opened = memory.opened or action.opener == true
    if action.debuff ~= nil and event.type == "DONE" then
        memory.applied                = memory.applied or {}
        memory.applied[action.debuff] = brain:observe(bot).now
    end
    memory.refused = event.type == "REFUSED" and action.key or nil
    if event.type == "STRUCK" or event.type == "DONE" or event.type == "REFUSED" then
        brain:remember(bot, "battle", memory)
    else
        brain:remember(bot, "battle", nil)
    end
    return event
end

return M
