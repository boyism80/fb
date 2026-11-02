local combat = require("brain.data.combat")
local items  = require("brain.data.items")
local magic  = require("brain.data.spells")
local trees  = require("brain.data.tree")
local bag    = require("brain.policies.bag")

local M = {}

local OFFENSE = { ATTACK = true, AREA = true }
local HITTING = { AGGRESSIVE = true, RETALIATE = true, STATIONARY = true }

local function truncate(value)
    return value < 0 and math.ceil(value) or math.floor(value)
end

local function resist(mob, kind)
    return (mob.resists or {})[kind] or 0
end

local function amount(value, me)
    if type(value) == "function" then
        return value(me)
    end
    return value or 0
end

function M.mitigate(value, defense, rate)
    local n       = truncate((100 - defense) / 10)
    local percent = -125 + (n * (2 * 14.75 - (n - 1) / 2.0)) / 2.0
    local damage  = value - truncate(percent * (value / 100.0))
    return math.max(0, damage * (1.0 - (rate or 0)))
end

function M.accuracy(hit)
    local rate = nil
    if hit >= combat.hit.front then
        rate = combat.hit.max
    elseif hit <= combat.hit.floor then
        rate = combat.hit.min
    else
        rate = combat.hit.min + math.floor((hit - combat.hit.floor) * (combat.hit.max - combat.hit.min) /
            (combat.hit.front - combat.hit.floor))
    end
    return rate / 100.0
end

local function weapon(snap, query)
    for _, slot in ipairs(snap.gear) do
        if slot.type == "WEAPON" and slot.names[1] ~= nil then
            return query.item(slot.names[1])
        end
    end
    return nil
end

function M.buffs(snap)
    local result = { rate = 0, hit = 0, dam = 0, phydef = 0, derate = 1, opener = 1, echo = 1, splash = 0, enchant = 0 }
    local ambush = false
    local echoes = {}
    local active = {}
    for _, name in ipairs(magic.echoes or {}) do
        echoes[name] = true
    end
    for _, name in ipairs(snap.buffs or {}) do
        active[name] = true
    end
    for _, learned in ipairs(snap.spells or {}) do
        local data  = magic.spells[learned.name]
        local boost = data ~= nil and data.boost or {}
        local stats = active[learned.name] and {} or boost
        if data ~= nil and data.role == "BUFF" then
            result.rate   = math.max(result.rate, (boost.damage_rate or 0) / 1000)
            result.hit    = result.hit + (stats.buff_hit or 0)
            result.dam    = result.dam + (stats.buff_dam or 0)
            result.phydef = result.phydef + (stats.buff_phydef or 0)
            result.derate = result.derate + (boost.damage_derate or 0) / 1000
            result.opener = math.max(result.opener, magic.multipliers[learned.name] or 1)
            if echoes[learned.name] then
                result.echo = result.echo + 1
            end
            local splash = (magic.splashes or {})[learned.name]
            if splash ~= nil then
                result.splash = result.splash + splash.rate * splash.sides
            end
            if data.trigger == "MANA" and data.surge ~= nil then
                result.surge = { rate = data.surge / (magic.buff_tick or 1), mp = data.mp or 0 }
            elseif data.trigger == "EMERGENCY" then
                result.emergency = { chance = data.chance or 1, time = (data.buff_time or 0) * 1000, mp = data.mp or 0 }
            end
        elseif data ~= nil and data.role == "AMBUSH" then
            ambush = true
        elseif data ~= nil and data.role == "ENCHANT" then
            result.enchant = math.max(result.enchant, data.damage or 0)
        elseif data ~= nil and data.role == "CHARGE" then
            result.charge = {
                chance = math.min(1, (data.chance or 0) + (data.chance_int or 0) * (snap.self.int or 0)),
                cost   = (data.hp_cost or 0) * (snap.self.max_mp or 0),
                mp     = data.mp or 0,
            }
        end
    end
    if ambush then
        result.opener = result.opener * magic.back_attack
    end
    return result
end

function M.debuffs(snap, mob)
    local result = { defense = 0, dot = 0, dot_time = 0, disable = 0, mp = 0, casts = 0 }
    for _, learned in ipairs(snap.spells or {}) do
        local data = magic.spells[learned.name]
        if data ~= nil and data.role == "DEBUFF" then
            local chance = 1
            for _, kind in ipairs(data.resists or {}) do
                chance = chance * (1 - resist(mob, kind))
            end
            local time = (data.buff_time or 0) * 1000 * chance
            local used = false
            if (data.stats or {}).buff_phydef ~= nil and data.stats.buff_phydef * chance > result.defense then
                result.defense = data.stats.buff_phydef * chance
                used           = true
            end
            if data.dot ~= nil and data.dot * chance > result.dot then
                result.dot      = data.dot * chance
                result.dot_time = (data.buff_time or 0) * 1000
                used            = true
            end
            if (data.paralysis or data.blind) and time > result.disable then
                result.disable = time
                used           = true
            end
            if used then
                result.mp    = result.mp + (data.mp or 0)
                result.casts = result.casts + 1
            end
        end
    end
    return result
end

function M.strike(snap, query, mob, density, weaken)
    local buffs = M.buffs(snap)
    local model = weapon(snap, query)
    local range = combat.bare
    local bonus = 0
    if model ~= nil and model.small ~= nil then
        range = mob.size == "LARGE" and model.large or model.small
        bonus = buffs.enchant
    end
    local base     = (math.max(1, range.min) + math.max(range.min, range.max)) / 2.0 + bonus
    local rate     = 1.0 + buffs.rate + ((snap.self.dam or 0) + buffs.dam) * combat.dam
    local critical = 1.0 + combat.critical.chance * (combat.critical.factor - 1)
    local defense  = (mob.defense or 0) + (weaken or 0)
    local spread   = 1 + buffs.splash * (density or 0)
    return M.accuracy((snap.self.hit or 0) + buffs.hit) * M.mitigate(base, defense, resist(mob, "PHYSICAL")) * rate *
        critical * buffs.echo * spread
end

function M.hits(mob)
    return HITTING[mob.attack] == true
end

function M.blow(snap, mob)
    if M.hits(mob) == false or mob.damage == nil then
        return 0
    end
    local buffs = M.buffs(snap)
    return M.mitigate(((mob.least or mob.damage) + mob.damage) / 2.0, (snap.self.phydef or 0) - buffs.phydef, 0) /
        buffs.derate
end

function M.options(snap, mob)
    local me     = { hp = snap.self.hp, mp = snap.self.mp or 0 }
    local result = { offense = {}, heal = {}, strike = {} }
    for _, learned in ipairs(snap.spells or {}) do
        local data = magic.spells[learned.name]
        if data ~= nil and OFFENSE[data.role] then
            local value = M.mitigate(amount(data.damage, me), mob.ward or 0, resist(mob, "MAGIC"))
            table.insert(result.offense, {
                name   = learned.name,
                damage = value * (1 - resist(mob, "SPELL")),
                mp     = amount(data.mp, me),
            })
        elseif data ~= nil and data.role == "HEAL" then
            table.insert(result.heal, {
                name   = learned.name,
                hp     = amount(data.hp, me),
                mp     = amount(data.mp, me),
                others = data.self ~= true,
            })
        elseif data ~= nil and data.role == "STRIKE" then
            local value = M.mitigate(amount(data.damage, me), mob.defense or 0, resist(mob, "PHYSICAL"))
            table.insert(result.strike, {
                name   = learned.name,
                damage = value,
                hp     = amount(data.hp, me),
                mp     = amount(data.mp, me),
            })
        end
    end
    return result
end

local function best(choices, mp, field)
    local found = nil
    for _, choice in ipairs(choices) do
        if choice.mp <= mp and choice[field] > 0 and (found == nil or choice[field] > found[field]) then
            found = choice
        end
    end
    return found
end

local function landed(time, disable, speed, blow)
    if speed == nil or speed <= 0 then
        return 0
    end
    return math.floor(math.max(0, time - disable) / speed) * blow
end

function M.fight(snap, query, mob, density)
    local cast  = 1000 / magic.cast_rate
    local mp    = snap.self.mp or 0
    local hp    = snap.self.hp
    local time  = 0
    local casts = 0
    local weak  = M.debuffs(snap, mob)
    if weak.mp > mp then
        weak = { defense = 0, dot = 0, dot_time = 0, disable = 0, mp = 0, casts = 0 }
    end
    mp    = mp - weak.mp
    time  = time + weak.casts * cast
    casts = casts + weak.casts

    local strike  = M.strike(snap, query, mob, density, weak.defense)
    local blow    = M.blow(snap, mob)
    local buffs   = M.buffs(snap)
    local options = M.options(snap, mob)
    local left    = mob.hp or 0
    local opener  = best(options.strike, mp, "damage")
    if opener ~= nil and hp - opener.hp > blow then
        left  = left - opener.damage
        mp    = mp - opener.mp
        hp    = hp - opener.hp
        time  = time + cast
        casts = casts + 1
    end
    local spell = best(options.offense, mp, "damage")
    while spell ~= nil and left > 0 do
        left  = left - spell.damage
        mp    = mp - spell.mp
        time  = time + cast
        casts = casts + 1
        spell = best(options.offense, mp, "damage")
    end
    if left > 0 and strike <= 0 and weak.dot <= 0 then
        return {
            swings = math.huge,
            casts = casts,
            time = math.huge,
            taken = math.huge,
            healed = 0,
            drained = 0,
            margin = -math.huge
        }
    end

    if left > 0 then
        left = left - strike * (buffs.opener - 1)
    end
    local swings = 0
    if left > 0 then
        local melee  = strike / combat.swing
        local poison = weak.dot * (mob.hp or 0) / ((magic.buff_tick or 1) * 1000)
        local during = math.min(weak.dot_time, left / math.max(melee + poison, 1e-9))
        local rest   = math.max(0, left - (melee + poison) * during)
        local after  = rest > 0 and (melee > 0 and rest / melee or math.huge) or 0
        swings       = math.ceil((during + after) * melee / math.max(strike, 1e-9))
        time         = time + during + after
    end
    local taken  = landed(time, weak.disable, mob.speed, blow)
    local healed = 0
    local heal   = best(options.heal, mp, "hp")
    while heal ~= nil and heal.mp > 0 and taken - healed >= hp do
        healed = healed + heal.hp
        mp     = mp - heal.mp
        time   = time + cast
        taken  = landed(time, weak.disable, mob.speed, blow)
        heal   = best(options.heal, mp, "hp")
    end
    local guard = buffs.emergency
    if guard ~= nil and taken - healed >= hp and mp >= guard.mp and mob.speed ~= nil and mob.speed > 0 then
        mp    = mp - guard.mp
        taken = math.max(0, taken - guard.chance * guard.time / mob.speed * blow)
    end
    return {
        swings  = swings,
        casts   = casts,
        time    = time,
        taken   = taken,
        healed  = healed,
        drained = (snap.self.mp or 0) - mp,
        margin  = (hp + healed - taken) / math.max(snap.self.max_hp, 1),
    }
end

function M.plan(snap, query, mob, guard)
    local options = M.options(snap, mob)
    local slots   = {}
    for _, learned in ipairs(snap.spells or {}) do
        slots[learned.name] = learned.slot
    end
    table.sort(options.offense, function(left, right)
        return left.damage > right.damage
    end)
    local skills = {}
    local least  = math.huge
    for _, choice in ipairs(options.offense) do
        if choice.damage > 0 then
            table.insert(skills, { slot = slots[choice.name], mp = choice.mp })
            least = math.min(least, choice.mp)
        end
    end
    local heal            = best(options.heal, math.huge, "hp")
    local potions, drinks = {}, {}
    local refill          = 0
    for name, count in pairs(bag.usable(snap)) do
        local effect = count > 0 and items[name] or nil
        if effect ~= nil and ((effect.hp or 0) > 0 or effect.hp_full or effect.hp_ratio) then
            table.insert(potions, name)
            refill = math.max(refill, effect.hp_full and snap.self.max_hp or
                (effect.hp_ratio or 0) * snap.self.max_hp + math.max(0, effect.hp or 0))
        end
        if effect ~= nil and ((effect.mp or 0) > 0 or effect.mp_full or effect.mp_ratio) then
            table.insert(drinks, name)
        end
    end
    local blow   = M.blow(snap, mob)
    local amount = heal ~= nil and heal.hp or refill
    local strike = best(options.strike, math.huge, "damage")
    local opener = nil
    if strike ~= nil then
        local cost = magic.spells[strike.name].hp_cost
        local floor = cost ~= nil and cost < 1 and blow / (1 - cost) or strike.hp + blow
        opener = { slot = slots[strike.name], mp = strike.mp, floor = math.ceil(floor) }
    end
    return {
        guard   = guard,
        skills  = skills,
        opener  = opener,
        heal    = heal ~= nil and { slot = slots[heal.name], mp = heal.mp } or nil,
        mend    = amount > 0 and math.max(snap.self.max_hp - amount, blow) or 0,
        potions = potions,
        drinks  = drinks,
        melee   = snap.self.class == "WARRIOR" or #skills == 0 or ((snap.self.mp or 0) < least and #drinks == 0),
        buffs   = {},
        tree    = trees.combat(),
    }
end

return M
