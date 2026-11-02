local data     = require("brain.data.combat")
local items    = require("brain.data.items")
local magic    = require("brain.data.spells")
local bag      = require("brain.policies.bag")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
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
    local options = combat.options(snap, mob)
    local known   = slots(snap)
    local cast    = 1000 / magic.cast_rate
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
    add({
        key = "SWING",
        kind = "SWING",
        time = swing,
        damage = combat.strike(snap, facts, mob),
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
        local after = copy(snap, { self = copy(snap.self, { hp = hp, mp = mp }) })
        local prey  = copy(mob, { ratio = left / math.max(mob.hp or 1, 1) })
        rest        = combat.outcome(after, facts, prey)
        ending      = survival.ending(after, rest, survival.contact(after, prey, rest.time))
    elseif hp <= 0 then
        rest = { time = 0, p_die = 1 }
    end
    local p_die = 1 - (1 - now) * (1 - rest.p_die)
    local time  = (entry.time + rest.time) / 1000
    local risk  = (1 - p_die) * combat.gain(mob) - p_die * loss
    return risk - time * worth.value - (entry.price or 0) * worth.exchange, ending > 0
end

local function remembered(snap, mob)
    local state  = { opened = false, refused = {} }
    local memory = snap.record.memory.battle
    if memory ~= nil and memory.oid == mob.oid then
        state.opened = memory.opened == true
        if memory.refused ~= nil then
            state.refused[memory.refused] = true
        end
    end
    return state
end

function M.choose(snap, facts, mob)
    local baseline = value.baseline(snap, facts)
    local loss     = danger.loss(snap, facts, baseline)
    local best     = nil
    local values   = {}
    for _, entry in ipairs(candidates(snap, facts, mob, remembered(snap, mob))) do
        entry.value, entry.survives = estimate(snap, facts, mob, entry, baseline, loss)
        table.insert(values, string.format("%s:%s=%.1f%s", entry.kind, entry.key, entry.value,
            entry.survives and "" or "!"))
        if entry.survives and (best == nil or entry.value > best.value) then
            best = entry
        end
    end
    log("debug", string.format("[brain:battle] bot=%s hp=%d mp=%d mob=%s %s picks %s", snap.self.name or "",
        snap.self.hp, snap.self.mp or 0, mob.name or "", table.concat(values, " "), best ~= nil and best.key or "none"))
    return best
end

function M.target(snap, facts, accept)
    local step  = snap.pace.step or data.walk
    local found = nil
    local best  = nil
    for _, mob in ipairs(snap.world.mobs) do
        if mob.x ~= nil and accept(mob) then
            local choice = M.choose(snap, facts, mob)
            if choice ~= nil then
                local distance = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
                local score    = choice.value - distance * step / 1000 * value.baseline(snap, facts).value
                if best == nil or score > best then
                    found = { mob = mob, choice = choice }
                    best  = score
                end
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
        opener = found.choice.opener == true
    }
    for key, entry in pairs(extra or {}) do
        action[key] = entry
    end
    return action
end

function M.step(brain, bot, oid, action)
    local memory = brain:recall(bot, "battle")
    if memory == nil or memory.oid ~= oid then
        memory = { oid = oid, opened = false }
    end
    local event    = brain:engage(bot, oid, action.maneuver)
    memory.opened  = memory.opened or action.opener == true
    memory.refused = event.type == "REFUSED" and action.key or nil
    if event.type == "STRUCK" or event.type == "DONE" or event.type == "REFUSED" then
        brain:remember(bot, "battle", memory)
    else
        brain:remember(bot, "battle", nil)
    end
    return event
end

return M
