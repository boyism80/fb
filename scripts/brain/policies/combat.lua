local balance = require("brain.data.balance")
local combat  = require("brain.data.combat")
local hunting = require("brain.data.hunting")
local magic   = require("brain.data.spells")

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

local boosts = setmetatable({}, { __mode = "k" })

function M.buffs(snap)
    if boosts[snap] ~= nil then
        return boosts[snap]
    end
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
    boosts[snap] = result
    return result
end

function M.debuffs(snap, mob)
    local result = { defense = 0, dot = 0, dot_time = 0, disable = 0, mp = 0, casts = 0 }
    local function apply(name, cost)
        local data = magic.spells[name]
        if data == nil or data.role ~= "DEBUFF" then
            return
        end
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
        if used and cost then
            result.mp    = result.mp + (data.mp or 0)
            result.casts = result.casts + 1
        end
    end
    for name in pairs(mob.applied or {}) do
        apply(name, false)
    end
    for name in pairs(mob.assist or {}) do
        apply(name, false)
    end
    if mob.applied == nil then
        for _, learned in ipairs(snap.spells or {}) do
            if (mob.assist or {})[learned.name] == nil then
                apply(learned.name, true)
            end
        end
    end
    return result
end

function M.lasting(name, at, now)
    local spell = magic.spells[name]
    return type(at) ~= "number" or spell == nil or now - at < (spell.buff_time or 0) * 1000
end

function M.exposed(mob, weak)
    if weak.defense == 0 then
        return mob
    end
    local result = {}
    for key, entry in pairs(mob) do
        result[key] = entry
    end
    result.defense = (mob.defense or 0) + weak.defense
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

function M.spread(snap, mob)
    if M.hits(mob) == false or mob.damage == nil then
        return 0, 0
    end
    local buffs   = M.buffs(snap)
    local defense = (snap.self.phydef or 0) - buffs.phydef
    return M.mitigate(mob.least or mob.damage, defense, 0) / buffs.derate,
        M.mitigate(mob.damage, defense, 0) / buffs.derate
end

function M.blow(snap, mob)
    local low, high = M.spread(snap, mob)
    return (low + high) / 2.0
end

local function normal(z)
    local t    = 1 / (1 + 0.3275911 * math.abs(z) / math.sqrt(2))
    local tail = t * (0.254829592 + t * (-0.284496736 + t * (1.421413741 + t * (-1.453152027 + t * 1.061405429)))) *
        math.exp(-z * z / 2) / 2
    return z >= 0 and 1 - tail or tail
end

function M.reaches(count, low, high, hp)
    if count <= 0 or high <= 0 then
        return 0
    elseif hp <= 0 or count == math.huge then
        return 1
    elseif count == 1 then
        return high <= low and (high >= hp and 1 or 0) or math.max(0, math.min(1, (high - hp) / (high - low)))
    end
    local mean  = count * (low + high) / 2
    local width = high - low
    local sd    = math.sqrt(count * width * width / 12)
    if sd <= 0 then
        return mean >= hp and 1 or 0
    end
    return 1 - normal((hp - mean) / sd)
end

function M.lethal(snap, mob, window, hp)
    local low, high = M.spread(snap, mob)
    local speed     = mob.speed
    if window <= 0 or high <= 0 or speed == nil or speed <= 0 then
        return 0
    elseif window == math.huge then
        return 1
    end
    local count = window / speed
    local whole = math.floor(count)
    local part  = count - whole
    return (1 - part) * M.reaches(whole, low, high, hp) + part * M.reaches(whole + 1, low, high, hp)
end

function M.gain(mob)
    local known = hunting.mobs[mob.id]
    return mob.exp or (known ~= nil and known.exp) or 0
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

local function volley(options, mp, left)
    local casts = 0
    local spell = best(options.offense, mp, "damage")
    while spell ~= nil and left > 0 do
        local count = math.ceil(left / spell.damage)
        if spell.mp > 0 then
            count = math.min(count, math.floor(mp / spell.mp))
        end
        left  = left - count * spell.damage
        mp    = mp - count * spell.mp
        casts = casts + count
        spell = best(options.offense, mp, "damage")
    end
    return left, mp, casts
end

function M.plain(options, mp, left, cast, melee)
    local casts = 0
    left, mp, casts = volley(options, mp, left)
    local time = casts * cast
    if left > 0 then
        if melee <= 0 then
            return math.huge
        end
        time = time + left / melee
    end
    return time
end

function M.opener(snap, mob)
    return best(M.options(snap, mob).strike, snap.self.mp or 0, "damage")
end

local function landed(time, disable, speed, blow)
    if speed == nil or speed <= 0 then
        return 0
    end
    return math.max(0, time - disable) / speed * blow
end

function M.interval()
    return 1000 / math.min(balance.cast_rate, magic.cast_rate)
end

function M.cast(snap)
    return math.max(M.interval(), (snap.pace or {}).cast or 0)
end

function M.cooldown(snap)
    local casted = snap.self.casted
    if casted == nil then
        return 0
    end
    return math.max(0, M.interval() - casted)
end

function M.casting(action)
    return action.kind == ACTION.CAST or (action.maneuver ~= nil and action.maneuver.kind == MANEUVER.CAST)
end

function M.outcome(snap, query, mob, density, aid)
    local cast  = M.cast(snap)
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

    local exposed = M.exposed(mob, weak)
    local strike  = M.strike(snap, query, exposed, density)
    local blow    = M.blow(snap, mob)
    local buffs   = M.buffs(snap)
    local options = M.options(snap, exposed)
    local left    = (mob.hp or 0) * (mob.ratio or 1)
    local opener  = best(options.strike, mp, "damage")
    local spent   = 0
    local rival   = left / math.max(M.plain(options, mp, left, cast, strike / combat.swing), 1e-9)
    local repays  = opener ~= nil and (opener.hp <= 0 or
        (aid or 0) > 0 and opener.damage / math.max(rival, 1e-9) >= opener.hp / aid)
    if opener ~= nil and repays and hp - opener.hp > blow then
        left  = left - opener.damage
        mp    = mp - opener.mp
        hp    = hp - opener.hp
        spent = opener.hp
        time  = time + cast
        casts = casts + 1
    end
    if (aid or 0) > 0 and opener ~= nil and opener.hp > 0 and opener.damage > 0 and left > 0 then
        local period = math.max(cast, opener.hp / aid)
        if opener.damage / period > rival then
            local count = math.ceil(left / opener.damage)
            if opener.mp > 0 then
                count = math.min(count, math.floor(mp / opener.mp))
            end
            left  = left - count * opener.damage
            mp    = mp - count * opener.mp
            time  = time + count * period
            casts = casts + count
        end
    end
    local volleys     = 0
    left, mp, volleys = volley(options, mp, left)
    time              = time + volleys * cast
    casts             = casts + volleys
    if left > 0 and strike <= 0 and weak.dot <= 0 then
        return {
            swings = math.huge,
            casts = casts,
            time = math.huge,
            taken = math.huge,
            healed = 0,
            drained = 0,
            margin = -math.huge,
            p_die = M.lethal(snap, mob, math.huge, hp),
            gain = 0
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
    local taken  = landed(time, weak.disable, mob.speed, blow) + spent
    local healed = 0
    local heal   = best(options.heal, mp, "hp")
    while heal ~= nil and heal.mp > 0 and taken - healed >= hp do
        healed = healed + heal.hp
        mp     = mp - heal.mp
        time   = time + cast
        taken  = landed(time, weak.disable, mob.speed, blow) + spent
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
        spent   = spent,
        healed  = healed,
        drained = (snap.self.mp or 0) - mp,
        margin  = (hp + healed - taken) / math.max(snap.self.max_hp, 1),
        p_die   = M.lethal(snap, mob, time - weak.disable, hp + healed),
        gain    = M.gain(mob),
    }
end

return M
