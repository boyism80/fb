local growth  = require("brain.data.growth")
local balance = require("brain.data.balance")
local combat  = require("brain.data.combat")
local hunting = require("brain.data.hunting")
local magic   = require("brain.data.spells")

local M = {}

local OFFENSE = { ATTACK = true, AREA = true }

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

function M.resistance(ward)
    return -math.max(-combat.defense_limit, math.min(combat.defense_limit, ward or 0))
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

function M.warding(name)
    local data  = magic.spells[name]
    local boost = data ~= nil and data.boost or {}
    return data ~= nil and data.role == "BUFF" and data.self == false and data.others == true and
        ((boost.damage_derate or 0) > 0 or (boost.buff_phydef or 0) > 0)
end

-- The wards kept up on me, by me or a crewmate: a ward cast on others with no reuse delay (무장, 보호) is counted on
-- me whether it is on this moment or not, as my own kept buffs are. Counted only while it was on, every cast and every
-- end of one changed the build the crew planned for, and the whole crew worked its plan out anew and held its walks
-- 1-3s each time (online 20261115-151147; user 11-16). A member's view carries the set its crew posted.
function M.kept(snap)
    if snap.kept ~= nil then
        return snap.kept
    end
    local found = {}
    local function add(spells)
        for _, learned in ipairs(spells or {}) do
            local data = magic.spells[learned.name]
            if M.warding(learned.name) and data.delay == nil then
                found[learned.name] = true
            end
        end
    end
    add(snap.spells)
    for _, profile in ipairs(snap.party or {}) do
        if profile.self == nil or profile.self.ghost ~= true then
            add(profile.spells)
        end
    end
    return found
end

local MODELED = { damage_rate = true, buff_hit = true, buff_dam = true, buff_phydef = true, damage_derate = true }

-- A buff the fight model counts: a boost it reads, a strike multiplier, a splash or an echo.
function M.valued(name)
    local data = magic.spells[name]
    if data == nil then
        return false
    end
    for key in pairs(data.boost or {}) do
        if MODELED[key] then
            return true
        end
    end
    if (magic.multipliers or {})[name] ~= nil or (magic.splashes or {})[name] ~= nil then
        return true
    end
    for _, echo in ipairs(magic.echoes or {}) do
        if echo == name then
            return true
        end
    end
    return false
end

local NONE = {}

-- A spell that comes off a die roll (금강불체: 30% on the live server, every cast on a debug build) is counted at the
-- odds this bot has seen, from the data's odds as one first try: held to the data's 0.3, a poet on a debug server
-- would weigh as a gamble the guard it always gets.
function M.tallied(rolls, name, hit)
    local result = {}
    for key, entry in pairs(type(rolls) == "table" and rolls or NONE) do
        result[key] = entry
    end
    local seen   = result[name] or { tries = 0, hits = 0 }
    result[name] = { tries = seen.tries + 1, hits = seen.hits + (hit and 1 or 0) }
    return result
end

function M.rolled(snap, name, prior)
    local roll = ((snap.record or {}).memory or {}).rolls
    local seen = type(roll) == "table" and roll[name] or nil
    return ((seen ~= nil and seen.hits or 0) + prior) / ((seen ~= nil and seen.tries or 0) + 1)
end

-- While a guard is on me (금강불체) no blow takes hp: what the mobs beside me deal is counted from the time it ends,
-- as the time a held mob cannot strike is (debuffs.disable), so the danger rises as the guard runs out instead of
-- coming back all at once.
function M.invincible(snap)
    local left = 0
    for name, ms in pairs(snap.buff_left or NONE) do
        local data = magic.spells[name]
        if data ~= nil and data.trigger == "EMERGENCY" then
            left = math.max(left, ms)
        end
    end
    return left
end

function M.pressure(snap, member, window)
    local total = 0
    if member.world.map ~= snap.world.map or member.world.x == nil then
        return total
    end
    local open = math.max(0, window - M.invincible(member))
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and M.hits(mob) and (mob.speed or 0) > 0 and
            math.abs(mob.x - member.world.x) + math.abs(mob.y - member.world.y) == 1 then
            local _, high = M.spread(member, mob)
            total = total + high * open / mob.speed
        end
    end
    return total
end

local function compute_buffs(snap)
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
        if data ~= nil and data.role == "BUFF" and data.self ~= false then
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
                result.emergency = {
                    chance = M.rolled(snap, learned.name, data.chance or 1),
                    time   = (data.buff_time or 0) * 1000,
                    mp     = data.mp or 0,
                }
            end
        elseif data ~= nil and data.role == "AMBUSH" then
            ambush = true
        elseif data ~= nil and data.role == "ENCHANT" then
            result.enchant = math.max(result.enchant, data.damage or 0)
        elseif data ~= nil and data.role == "CHARGE" then
            result.charge = {
                chance = math.min(1, (data.chance or 0) + (data.chance_int or 0) * (snap.self.int or 0)),
                cost   = (data.hp_cost or 0) * (snap.self.max_mp or 0),
                floor  = data.hp_floor or 0,
                mp     = data.mp or 0,
            }
        end
    end
    if ambush then
        result.opener = result.opener * magic.back_attack
    end
    local wards = {}
    for _, name in ipairs(snap.aided or {}) do
        wards[name] = active[name] and "ACTIVE" or "PLANNED"
    end
    for name in pairs(active) do
        local data = magic.spells[name]
        if data ~= nil and data.role == "BUFF" and data.self == false then
            wards[name] = "ACTIVE"
        end
    end
    for _, learned in ipairs(snap.spells or {}) do
        if M.warding(learned.name) then
            wards[learned.name] = wards[learned.name] or "PLANNED"
        end
    end
    for name in pairs(M.kept(snap)) do
        wards[name] = wards[name] or "PLANNED"
    end
    for name, state in pairs(wards) do
        local boost = (magic.spells[name] or {}).boost or {}
        result.derate = result.derate + (boost.damage_derate or 0) / 1000
        if state == "PLANNED" then
            result.phydef = result.phydef + (boost.buff_phydef or 0)
        end
    end
    result.worn = 1
    for name in pairs(active) do
        local boost = (magic.spells[name] or {}).boost or {}
        result.worn = result.worn + (boost.damage_derate or 0) / 1000
    end
    return result
end

function M.warded(buffs, name)
    for _, active in ipairs(buffs or {}) do
        if active:sub(- #name) == name then
            return true
        end
    end
    return false
end

function M.buffs(snap)
    local spells = snap.spells or NONE
    local bybuff = boosts[spells]
    if bybuff == nil then
        bybuff = setmetatable({}, { __mode = "k" })
        boosts[spells] = bybuff
    end
    local buffs = snap.buffs or NONE
    local byaid = bybuff[buffs]
    if byaid == nil then
        byaid = setmetatable({}, { __mode = "k" })
        bybuff[buffs] = byaid
    end
    local aided = snap.aided or NONE
    local found = byaid[aided]
    if found == nil or found.owner_int ~= snap.self.int or found.owner_max ~= snap.self.max_mp then
        found = compute_buffs(snap)
        found.owner_int = snap.self.int
        found.owner_max = snap.self.max_mp
        found.spreads = {}
        byaid[aided] = found
    end
    return found
end

-- The hp a poison's ticks leave a mob (dot_floor) as its bar shows it: the bar counts whole percents, so a mob with
-- more than a hundred times that hp shows its last percent when the ticks stop. Counted at the floor alone, a 흑해골
-- ticked down to 100 of 20,000 showed 1%, looked to hold 100 more for the ticks, and was poisoned again 14 times on
-- one floor (online 20261115-185734, 흑해골굴).
function M.floor(mob, spell)
    return math.max(spell.dot_floor or 0, (mob.hp or 0) / 100)
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

function M.marks(snap, oid)
    local names = {}
    local add   = function(marks)
        for name, at in pairs((marks or {})[tostring(oid)] or {}) do
            if M.lasting(name, at, snap.now) then
                names[name] = true
                for _, group in ipairs(combat.debuffs) do
                    local inside = false
                    for _, member in ipairs(group) do
                        inside = inside or member == name
                    end
                    for _, member in ipairs(inside and group or {}) do
                        names[member] = true
                    end
                end
            end
        end
    end
    add(snap.record.memory.marks)
    for _, profile in ipairs(snap.party or {}) do
        add(profile.marks)
    end
    return names
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

function M.strike(snap, query, mob, density, weaken, extra)
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
    local occupied = extra ~= nil and math.min(1, extra / math.max(combat.adjacent - 1, 1)) or (density or 0)
    local spread   = 1 + buffs.splash * occupied
    return M.accuracy((snap.self.hit or 0) + buffs.hit) * M.mitigate(base, defense, resist(mob, "PHYSICAL")) * rate *
        critical * buffs.echo * spread
end

function M.attack(mob)
    return combat.attacks[mob.attack] or combat.attacks.NONE
end

function M.hits(mob)
    return M.attack(mob).hits
end

function M.seeks(mob)
    return M.attack(mob).seeks
end

function M.striking(mob)
    local ratio = mob.ratio or 1
    return ratio > 0 and math.max(0, ratio - M.attack(mob).flee) / ratio or 0
end

function M.spread(snap, mob)
    if M.hits(mob) == false or mob.damage == nil then
        return 0, 0
    end
    local buffs   = M.buffs(snap)
    local defense = (snap.self.phydef or 0) - buffs.phydef
    local least   = mob.least or mob.damage
    local table_  = buffs.spreads[defense]
    if table_ == nil then
        table_ = {}
        buffs.spreads[defense] = table_
    end
    local key   = least * 1000003 + mob.damage
    local found = table_[key]
    if found == nil then
        found = { M.mitigate(least, defense, 0) / buffs.derate, M.mitigate(mob.damage, defense, 0) / buffs.derate }
        table_[key] = found
    end
    return found[1], found[2]
end

local function worn_spread(snap, mob)
    if M.hits(mob) == false or mob.damage == nil then
        return 0, 0
    end
    local worn = M.buffs(snap).worn
    return M.mitigate(mob.least or mob.damage, snap.self.phydef or 0, 0) / worn,
        M.mitigate(mob.damage, snap.self.phydef or 0, 0) / worn
end

local function typical(low, high)
    return (low + high) / 2.0 * (1 + combat.critical.chance * (combat.critical.factor - 1))
end

function M.imminent(snap, attackers)
    local open = math.max(0, M.cast(snap) - M.invincible(snap))
    if open <= 0 then
        return 0
    end
    local behind = ((magic.back_attack or 1) - 1) / math.max(combat.adjacent - 1, 1)
    local worst  = 0
    local flow   = 0
    local count  = 0
    for _, attacker in ipairs(attackers) do
        count = count + attacker.count
    end
    local back = count > 1 and (magic.back_attack or 1) or 1
    for _, attacker in ipairs(attackers) do
        local low, high = worn_spread(snap, attacker.mob)
        worst = math.max(worst, high * combat.critical.factor * back)
        if (attacker.mob.speed or 0) > 0 then
            flow = flow + attacker.count * typical(low, high) / attacker.mob.speed * (1 + behind)
        end
    end
    return worst + flow * open
end

function M.spike(snap, attackers)
    local open = math.max(0, M.cast(snap) - M.invincible(snap))
    if open <= 0 then
        return 0
    end
    local behind = ((magic.back_attack or 1) - 1) / math.max(combat.adjacent - 1, 1)
    local worst  = 0
    local flow   = 0
    local count  = 0
    for _, attacker in ipairs(attackers) do
        count = count + attacker.count
    end
    local back = count > 1 and (magic.back_attack or 1) or 1
    for _, attacker in ipairs(attackers) do
        local _, high = M.spread(snap, attacker.mob)
        worst = math.max(worst, high * combat.critical.factor * back)
        if (attacker.mob.speed or 0) > 0 then
            flow = flow + attacker.count * M.blow(snap, attacker.mob) / attacker.mob.speed * (1 + behind)
        end
    end
    return worst + flow * open
end

function M.blow(snap, mob)
    local low, high = M.spread(snap, mob)
    return (low + high) / 2.0 * (1 + combat.critical.chance * (combat.critical.factor - 1))
end

local function normal(z)
    local t    = 1 / (1 + 0.3275911 * math.abs(z) / math.sqrt(2))
    local tail = t * (0.254829592 + t * (-0.284496736 + t * (1.421413741 + t * (-1.453152027 + t * 1.061405429)))) *
        math.exp(-z * z / 2) / 2
    return z >= 0 and 1 - tail or tail
end

local function above(low, high, hp)
    if high <= low then
        return high >= hp and 1 or 0
    end
    return math.max(0, math.min(1, (high - hp) / (high - low)))
end

function M.reaches(count, low, high, hp)
    if count <= 0 or high <= 0 then
        return 0
    elseif hp <= 0 or count == math.huge then
        return 1
    end
    local chance = combat.critical.chance
    local factor = combat.critical.factor
    local single = (1 - chance) * above(low, high, hp) + chance * above(low * factor, high * factor, hp)
    if count <= 1 then
        return count * single
    end
    local mean  = count * (low + high) / 2 * (1 + chance * (factor - 1))
    local width = high - low
    local sd    = math.sqrt(count * width * width / 12)
    local total = sd > 0 and 1 - normal((hp - mean) / sd) or (mean >= hp and 1 or 0)
    return math.max(total, 1 - (1 - single) ^ count)
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

function M.capped(snap, exp)
    local levels = growth[snap.self.class]
    local now    = levels ~= nil and levels[snap.self.level] or nil
    if now == nil or levels[snap.self.level + 1] == nil then
        return exp
    end
    return math.min(exp, ((levels[snap.self.level + 1].exp - now.exp) / 100.0 * combat.exp_limit + 1) // 1)
end

function M.can_cast(snap)
    return snap.self.riding ~= true and snap.world.no_spell ~= true
end

function M.gain(mob, snap)
    local known = hunting.mobs[mob.id]
    local exp   = mob.exp or (known ~= nil and known.exp) or 0
    if snap == nil then
        return exp
    end
    return M.capped(snap, exp / (1 + #(snap.party or {}))) * (1 + #(snap.party or {}))
end

function M.surrounding(extra)
    return 1 + math.min(combat.adjacent - 1, extra or 0)
end

local choices = setmetatable({}, { __mode = "k" })

local function compute(snap, mob, extra)
    local me     = { hp = snap.self.hp, mp = snap.self.mp or 0 }
    local result = { offense = {}, heal = {}, strike = {} }
    for _, learned in ipairs(snap.spells or {}) do
        local data = magic.spells[learned.name]
        if data ~= nil and OFFENSE[data.role] and (data.limit == nil or (mob.hp or 0) <= data.limit) then
            local value = M.mitigate(amount(data.damage, me), M.resistance(mob.ward), resist(mob, "MAGIC"))
            local hits  = data.role == "AREA" and data.type == "NORMAL" and M.surrounding(extra) or 1
            table.insert(result.offense, {
                name   = learned.name,
                damage = value * (1 - resist(mob, "SPELL")) * hits,
                mp     = amount(data.mp, me),
                hp     = -(data.hp_spend or 0),
                front  = data.front == true,
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

function M.options(snap, mob, extra)
    local spells = snap.spells
    if spells == nil then
        return compute(snap, mob, extra)
    end
    local byspell = choices[spells]
    if byspell == nil then
        byspell = setmetatable({}, { __mode = "k" })
        choices[spells] = byspell
    end
    local bymob = byspell[mob.id or mob]
    if bymob == nil then
        bymob = {}
        byspell[mob.id or mob] = bymob
    end
    local stats   = string.format("%s|%s|%s", tostring(mob.defense), tostring(mob.ward), tostring(extra))
    local byextra = bymob[stats]
    if byextra == nil then
        byextra = {}
        bymob[stats] = byextra
    end
    -- Kept by the exact HP and MP: a spell whose damage follows HP (필살검무) came out for 2613.56 where 2613.6 was
    -- asked while the two shared a whole-number key, so which one was kept hung on which was asked first.
    local byhp = byextra[snap.self.hp]
    if byhp == nil then
        byhp = {}
        byextra[snap.self.hp] = byhp
    end
    local mp    = snap.self.mp or 0
    local found = byhp[mp]
    if found == nil then
        found = compute(snap, mob, extra)
        byhp[mp] = found
    end
    return found
end

local function best(choices, mp, field, hp)
    local found = nil
    for _, choice in ipairs(choices) do
        local spend = -(choice.hp or 0)
        if choice.mp <= mp and choice[field] > 0 and (hp == nil or spend <= 0 or spend < hp) and
            (found == nil or choice[field] > found[field]) then
            found = choice
        end
    end
    return found
end

-- A fight is worked out with the spells cast over and over. One that spends all the mana (헬파이어) is cast now and
-- then, between its delays and refills (user 11-15: the hunt goes on as it would without it meanwhile), so it is left
-- out here and weighed as a strike of its own when it can be cast (battle.candidates). Taken as the strongest spell,
-- a fight was one cast of it and blows after, the mana gone.
local function sustained(options)
    if options.sustained == nil then
        local found = {}
        for _, choice in ipairs(options.offense) do
            local spell = magic.spells[choice.name]
            if spell == nil or spell.drain ~= true then
                table.insert(found, choice)
            end
        end
        options.sustained = found
    end
    return options.sustained
end

local function volley(options, mp, left, hp)
    local casts   = 0
    local spent   = 0
    local offense = sustained(options)
    local spell   = best(offense, mp, "damage", hp)
    while spell ~= nil and left > 0 do
        local count = math.ceil(left / spell.damage)
        local spend = -(spell.hp or 0)
        if spell.mp > 0 then
            count = math.min(count, math.floor(mp / spell.mp))
        end
        if spend > 0 then
            count = math.min(count, math.ceil(hp / spend) - 1)
        end
        left  = left - count * spell.damage
        mp    = mp - count * spell.mp
        hp    = hp - count * math.max(spend, 0)
        spent = spent + count * math.max(spend, 0)
        casts = casts + count
        spell = count > 0 and best(offense, mp, "damage", hp) or nil
    end
    return left, mp, casts, spent
end

-- Nothing in this model gives back the hp a spell costs: its heals are paid from mp, and crew support is credited
-- only after the fight by survival. Counting a spell paid with hp against eating a dose every cast planned 40
-- 금강퇴 per mob for a lv81 rogue and sent it to 인어장군굴 (run 20261109-121438); counting it against the crew
-- aid planned a poet's hp down to nothing and charged under three skeletons (state charge_under_attack).
function M.bloodless(options)
    local offense = {}
    for _, choice in ipairs(options.offense) do
        if (choice.hp or 0) >= 0 then
            table.insert(offense, choice)
        end
    end
    return { offense = offense, heal = options.heal, strike = options.strike }
end

function M.plain(options, mp, left, cast, melee, hp)
    local casts = 0
    left, mp, casts = volley(options, mp, left, hp)
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

local function landed(time, disable, speed, blow, mob)
    if speed == nil or speed <= 0 then
        return 0
    end
    local delay = mob ~= nil and mob.attack == "RETALIATE" and speed / 2 or 0
    return math.max(0, time - disable - delay) * M.striking(mob) / speed * blow
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

function M.backswing(snap)
    local swung = snap.self.swung
    if swung == nil then
        return 0
    end
    return math.max(0, math.min(combat.swing, combat.swing - swung))
end

function M.swinging(snap, action)
    if action.kind ~= ACTION.ATTACK or action.maneuver == nil or action.maneuver.kind ~= MANEUVER.SWING then
        return false
    end
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.oid == action.oid and mob.x ~= nil then
            return math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1
        end
    end
    return false
end

function M.casting(action)
    return action.kind == ACTION.CAST or (action.maneuver ~= nil and action.maneuver.kind == MANEUVER.CAST)
end

local outcomes = setmetatable({}, { __mode = "k" })

-- Debuffs are kept by their names. A mob with no word of what is on it has my own debuffs planned against it and one
-- known to bear none has not, so the two keep apart: kept as one, whichever was asked first answered both, and a
-- warrior chose 웅담 or a swing on the same mob by whether the danger checks had fought it first (offline audit, run
-- 17:06). A crew's table is no key either: it goes with its crew and its address comes back for another.
local function names_key(names)
    if names == nil then
        return "?"
    end
    local sorted = {}
    for name in pairs(names) do
        sorted[#sorted + 1] = name
    end
    table.sort(sorted)
    return table.concat(sorted, ",")
end

local function compute_outcome(snap, query, mob, density, aid, extra)
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
    local strike  = mob.walled ~= true and M.strike(snap, query, exposed, density, nil, extra) or 0
    local blow    = M.blow(snap, mob)
    local buffs   = M.buffs(snap)
    local melee   = math.max(strike / combat.swing,
        buffs.opener > 1 and strike * buffs.opener / (2 * cast + combat.swing) or 0)
    local options = M.bloodless(M.options(snap, exposed, extra))
    local left    = (mob.hp or 0) * (mob.ratio or 1)
    local opener  = mob.walled ~= true and best(options.strike, mp, "damage") or nil
    local spent   = 0
    local rival   = left / math.max(M.plain(options, mp, left, cast, melee, hp), 1e-9)
    -- A strike saves only the time its damage takes off the mob, never more than the hp the mob has left: counted
    -- whole, the less hp a mob had left the surer its 건곤대나이 paid, and a warrior spent 3179 hp on a mob one spell
    -- ended (brain unit test, warrior63 at a twentieth of a mob's hp).
    local repays  = opener ~= nil and (opener.hp <= 0 or
        (aid or 0) > 0 and math.min(opener.damage, left) / math.max(rival, 1e-9) >= opener.hp / aid)
    if opener ~= nil and repays and hp - opener.hp > blow then
        left  = left - opener.damage
        mp    = mp - opener.mp
        hp    = hp - opener.hp
        spent = opener.hp
        time  = time + cast
        casts = casts + 1
    end
    local fills = 0
    if (aid or 0) > 0 and opener ~= nil and opener.hp > 0 and opener.damage > 0 and left > 0 then
        local period = math.max(cast, opener.hp / aid)
        local cycle  = opener.damage + melee * (period - cast)
        -- The last strike is cast whole and the swings after it last only as long as the hp it left: counted as a part
        -- of a cycle, a strike that ended the mob took less time than the cast, so a mob with less hp left fought longer
        -- and a rogue among foxes valued its 투명 as a swing still on its cooldown (offline profile trio seed 2, 733s,
        -- run 15:06).
        if cycle / period > rival then
            local count = left / cycle
            if opener.mp > 0 then
                count = math.min(count, math.floor(mp / opener.mp))
            end
            local whole = math.floor(count)
            local last  = math.ceil(count) > whole and left - whole * cycle or 0
            local after = math.max(0, last - opener.damage) / math.max(melee, 1e-9)
            left        = math.max(0, left - whole * cycle - last)
            mp          = math.max(0, mp - math.ceil(count) * opener.mp)
            time        = time + whole * period + (last > 0 and cast or 0) + after
            casts       = casts + math.ceil(count)
            fills       = (whole * (period - cast) + after) * melee / math.max(strike, 1e-9)
        end
    end
    local volleys           = 0
    local bled              = 0
    left, mp, volleys, bled = volley(options, mp, left, hp)
    spent                   = spent + bled
    time                    = time + volleys * cast
    casts                   = casts + volleys
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
        local poison = weak.dot * (mob.hp or 0) / ((magic.buff_tick or 1) * 1000)
        local during = math.min(weak.dot_time, left / math.max(melee + poison, 1e-9))
        local rest   = math.max(0, left - (melee + poison) * during)
        local after  = rest > 0 and (melee > 0 and rest / melee or math.huge) or 0
        swings       = math.ceil((during + after) * melee / math.max(strike, 1e-9))
        time         = time + during + after
    end
    -- The hp the strike cost is off hp already, so only the blows count against it: counted again within taken, a
    -- warrior that ended its mob with 건곤대나이 planned ten heals after the fight (brain unit test, warrior63).
    local guarded = M.invincible(snap)
    local spared  = math.max(weak.disable, guarded)
    local taken   = landed(time, spared, mob.speed, blow, mob) + spent
    local healed  = 0
    local heal    = best(options.heal, mp, "hp")
    while heal ~= nil and heal.mp > 0 and taken - spent - healed >= hp do
        healed = healed + heal.hp
        mp     = mp - heal.mp
        time   = time + cast
        taken  = landed(time, spared, mob.speed, blow, mob) + spent
        heal   = best(options.heal, mp, "hp")
    end
    local guard = buffs.emergency
    if guard ~= nil and guarded <= 0 and taken - spent - healed >= hp and mp >= guard.mp and mob.speed ~= nil and
        mob.speed > 0 then
        mp    = mp - guard.mp
        taken = math.max(0, taken - guard.chance * guard.time / mob.speed * blow)
    end
    return {
        swings  = swings + fills,
        casts   = casts,
        time    = time,
        taken   = taken,
        spent   = spent,
        healed  = healed,
        drained = (snap.self.mp or 0) - mp,
        margin  = (hp + spent + healed - taken) / math.max(snap.self.max_hp, 1),
        p_die   = M.lethal(snap, mob, time - spared, hp + healed),
        gain    = M.gain(mob, snap),
    }
end


function M.outcome(snap, query, mob, density, aid, extra)
    local gear = snap.gear
    if gear == nil then
        return compute_outcome(snap, query, mob, density, aid, extra)
    end
    local buffs  = M.buffs(snap)
    local bygear = outcomes[buffs]
    if bygear == nil then
        bygear = setmetatable({}, { __mode = "k" })
        outcomes[buffs] = bygear
    end
    local known = bygear[gear]
    if known == nil then
        known = {}
        bygear[gear] = known
    end
    local key = string.format("%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s", tostring(mob.id or mob.name or mob), snap.self.hp,
        snap.self.mp or 0, tostring(density), tostring(aid), tostring(extra), mob.ratio or 1, names_key(mob.applied),
        names_key(mob.assist), snap.self.phydef or 0, mob.hp or 0, tostring(mob.walled))
    local found = known[key]
    if found == nil then
        found = compute_outcome(snap, query, mob, density, aid, extra)
        known[key] = found
    end
    return found
end

return M
