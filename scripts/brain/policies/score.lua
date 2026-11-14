local combat   = require("brain.data.combat")
local items    = require("brain.data.items")
local magic    = require("brain.data.spells")
local teleport = require("brain.data.teleport")
local bag      = require("brain.policies.bag")
local fight    = require("brain.policies.combat")
local recovery = require("brain.policies.recovery")

local M = {}

local NEUTRAL = { defense = 0, ward = 0, resists = {}, size = "SMALL" }

local function sustained(choice, field, regen, rate, recovered)
    if choice == nil or choice[field] <= 0 then
        return 0
    end
    local paced = choice[field] * rate
    local spend = -(choice.hp or 0)
    if spend > 0 and field == "damage" then
        paced = math.min(paced, choice[field] * (recovered or 0) / spend)
    end
    if choice.mp <= 0 then
        return paced
    end
    return math.min(paced, choice[field] * regen / choice.mp)
end

local function affordable(choice, mp)
    if choice == nil then
        return 0
    end
    return choice.mp > 0 and choice.hp * (mp // choice.mp) or math.huge
end

local function strongest(choices, field, accept)
    local found = nil
    for _, choice in ipairs(choices) do
        if accept(choice) and (found == nil or choice[field] > found[field]) then
            found = choice
        end
    end
    return found
end

local function supplies(snap)
    local result = { hp = 0, mp = 0, money = snap.self.money or 0 }
    for name, count in pairs(bag.usable(snap)) do
        local effect = items[name]
        if effect ~= nil then
            local hp = effect.hp_full and snap.self.max_hp or
                (effect.hp_ratio or 0) * snap.self.max_hp + (effect.hp or 0)
            local mp = effect.mp_full and (snap.self.max_mp or 0) or
                (effect.mp_ratio or 0) * (snap.self.max_mp or 0) + (effect.mp or 0)
            result.hp = result.hp + math.max(0, hp) * count
            result.mp = result.mp + math.max(0, mp) * count
        end
    end
    return result
end

local function mobility(snap)
    local held  = {}
    local found = {}
    for _, spell in ipairs(snap.spells or {}) do
        held[spell.name] = true
    end
    for _, item in ipairs(snap.bag.items) do
        held[item.name] = true
    end
    for _, means in ipairs(teleport.spells) do
        if held[means.name] then
            table.insert(found, means.name)
        end
    end
    for _, means in ipairs(teleport.items) do
        if held[means.name] then
            table.insert(found, means.name)
        end
    end
    return found
end

local recovers = setmetatable({}, { __mode = "k" })
local WEARABLE = { WEAPON = true, ARMOR = true, HELMET = true, RING = true, SHIELD = true, AUXILIARY = true }

function M.recovered(snap, query)
    local death = snap.record ~= nil and snap.record.death or nil
    if death == nil or (death.left or 0) <= 0 then
        return snap
    end
    if recovers[snap] ~= nil then
        return recovers[snap]
    end
    local counts = {}
    local items  = {}
    for _, item in ipairs(snap.bag.items) do
        counts[item.name] = (counts[item.name] or 0) + item.count
        table.insert(items, item)
    end
    for name, count in pairs(death.items or {}) do
        local lacking = count - (counts[name] or 0)
        if lacking > 0 then
            table.insert(items, { name = name, count = lacking })
        end
    end
    local self = {}
    for key, entry in pairs(snap.self) do
        self[key] = entry
    end
    self.money = math.max(snap.self.money or 0, death.money or 0)
    local held = {}
    for key, entry in pairs(snap.bag) do
        held[key] = entry
    end
    held.items = items
    local worn = {}
    local used = {}
    for _, slot in ipairs(snap.gear or {}) do
        table.insert(worn, slot)
        used[slot.type] = true
    end
    for name in pairs(death.items or {}) do
        local model = query ~= nil and query.item(name) or nil
        local kind  = model ~= nil and model.type or nil
        if kind ~= nil and WEARABLE[kind] and used[kind] == nil and (counts[name] or 0) == 0 then
            table.insert(worn, { type = kind, names = { name } })
            used[kind] = true
        end
    end
    local result = setmetatable({ self = self, bag = held, gear = worn }, { __index = snap })
    recovers[snap] = result
    return result
end

local trimmed = setmetatable({}, { __mode = "k" })

-- Asked dry, a plan keeps only the self buffs the bot's own mana income pays for, cheapest first: counted kept up with
-- no mana at all, a dry plan was as good as a refilled one and no mana potion ever paid for a buff. Only a plan asked
-- dry drops them: rested as it stands, a bot that could buy no mana dropped the buff it was weighing and never cast it
-- (brain unit test, pure state 18).
local function kept(snap)
    local income  = recovery.rates(snap).mp
    local dropped = {}
    local paid    = 0
    for _, entry in ipairs(recovery.upkeeps(snap)) do
        if paid + entry.cost <= income then
            paid = paid + entry.cost
        else
            table.insert(dropped, entry.name)
        end
    end
    if #dropped == 0 or snap.spells == nil then
        return snap.spells
    end
    local key    = table.concat(dropped, ",")
    local byself = trimmed[snap.spells]
    if byself == nil then
        byself = {}
        trimmed[snap.spells] = byself
    end
    if byself[key] == nil then
        local skip  = {}
        local found = {}
        for _, name in ipairs(dropped) do
            skip[name] = true
        end
        for _, spell in ipairs(snap.spells) do
            if skip[spell.name] == nil then
                table.insert(found, spell)
            end
        end
        byself[key] = found
    end
    return byself[key]
end

function M.rested(snap, mode)
    local state  = mode or recovery.reserve(snap)
    local mp     = state == "REFILL" and recovery.ceiling(snap) or 0
    local full   = setmetatable({ hp = snap.self.max_hp, mp = mp }, { __index = snap.self })
    local spells = mode == "DRY" and kept(snap) or snap.spells
    return setmetatable({ self = full, mode = state, spells = spells }, { __index = snap })
end

local signatures = setmetatable({}, { __mode = "k" })

local compute_signature = nil

function M.signature(snap, mode)
    local known = signatures[snap]
    if known == nil then
        known = {}
        signatures[snap] = known
    end
    local key   = mode or false
    local found = known[key]
    if found == nil then
        found = compute_signature(snap, mode)
        known[key] = found
    end
    return found
end

compute_signature = function(snap, mode)
    local gear = {}
    for _, slot in ipairs(snap.gear) do
        table.insert(gear, table.concat(slot.names, "+"))
    end
    local active = {}
    for _, name in ipairs(snap.buffs or {}) do
        active[name] = true
    end
    local spells = {}
    local phydef = snap.self.phydef or 0
    local hit    = snap.self.hit or 0
    local dam    = snap.self.dam or 0
    for _, spell in ipairs(snap.spells or {}) do
        table.insert(spells, spell.name)
        local data = magic.spells[spell.name]
        if data ~= nil and data.role == "BUFF" and active[spell.name] then
            local stats = data.stats or {}
            phydef      = phydef - (stats.buff_phydef or 0)
            hit         = hit - (stats.buff_hit or 0)
            dam         = dam - (stats.buff_dam or 0)
        end
    end
    return table.concat({ snap.self.level, snap.self.max_hp, snap.self.max_mp or 0, phydef, hit,
        dam, table.concat(gear, ","), table.concat(spells, ","), mode or recovery.reserve(snap) }, "|")
end

function M.capability(snap, query, extra)
    local options   = fight.options(snap, NEUTRAL)
    local refill    = recovery.rates(snap)
    local regen     = refill.mp
    local recovered = refill.hp
    local rate      = 1000 / fight.cast(snap)
    local swing     = fight.strike(snap, query, NEUTRAL, nil, nil, extra)
    local melee     = swing * 1000 / combat.swing
    local single    = strongest(options.offense, "damage", function(choice)
        return magic.spells[choice.name].role == "ATTACK"
    end)
    local area      = strongest(options.offense, "damage", function(choice)
        return magic.spells[choice.name].role == "AREA"
    end)
    local heal      = strongest(options.heal, "hp", function()
        return true
    end)
    local others    = strongest(options.heal, "hp", function(choice)
        return choice.others
    end)
    local opener    = strongest(options.strike, "damage", function()
        return true
    end)
    local nuke      = single ~= nil and single.mp > 0 and math.floor((snap.self.mp or 0) / single.mp) * single.damage or
        0
    local guard     = fight.mitigate(100, snap.self.phydef or 0, 0) / 100

    return {
        dps      = {
            single = melee + sustained(single, "damage", regen, rate, recovered),
            area   = sustained(area, "damage", regen, rate, recovered),
            melee  = melee,
        },
        burst    = math.max(opener ~= nil and opener.damage or 0, nuke),
        blow     = {
            adjacent = math.max(swing, opener ~= nil and opener.damage or 0),
            ranged   = single ~= nil and single.damage or 0,
            mana     = single ~= nil and single.mp or 0,
            strike   = opener ~= nil and opener.damage or 0,
            heal     = others ~= nil and others.hp or 0,
        },
        ehp      = snap.self.hp / guard,
        hps      = {
            self   = sustained(heal, "hp", regen, rate),
            others = sustained(others, "hp", regen, rate),
            stock  = affordable(others, snap.self.mp or 0),
        },
        supply   = supplies(snap),
        refill   = refill,
        mobility = mobility(snap),
    }
end

-- A bot fights from a mob's side when nothing it does from afar outdoes its blow there, a heal on a crewmate counted
-- by the hp it gives. Told by the attack spell alone, a poet whose swing outdid its 화염주 was one that stays beside
-- its mob: its swing at 330 and the spell at 320, a level 9x poet held where the skeleton on it struck and swung,
-- struck 724s over three 15-minute seeds (offline WA+PO 흉가, run 20:18). What it keeps up counts as its one blow
-- does, and it is told rested, as its kind does not turn with its hp: by the one blow, a level 14 warrior, its swing
-- at 14 and 금강퇴 at 120, was no striker, and stepping off their mobs nine such warriors killed 4,974 for 5,861; a
-- level 99 rogue was one only while its hp kept 필살검무 above its spell and killed 724 for 1,292 (offline solos,
-- runs 21:06 and 19:16).
function M.melee(snap, query)
    local strong = M.capability(M.rested(snap), query)
    local blow   = strong.blow
    local afar   = strong.dps.single - strong.dps.melee
    return blow.adjacent >= math.max(blow.ranged, blow.heal) or strong.dps.melee >= math.max(afar, strong.hps.others)
end

-- A crewmate that heals the others is one whose heals on them outdo its own damage, told rested as its kind does not
-- turn with its hp. Told by any heal for the others, a rogue that healed 457 a second and dealt 9,551 counted as one,
-- left the prey it would walk to alone and stood four tiles behind its warrior for 28s (online trio 113100, 흑해골굴2).
function M.healer(snap, query)
    local strong = M.capability(M.rested(snap), query)
    return strong.hps.others > strong.dps.single
end

return M
