local data     = require("brain.data.combat")
local teleport = require("brain.data.teleport")
local battle   = require("brain.policies.battle")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local ground   = require("brain.policies.ground")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local score    = require("brain.policies.score")
local value    = require("brain.policies.value")

local M = {}

local pressed = setmetatable({}, { __mode = "k" })

local function pressing(snap, query)
    if pressed[snap] == nil then
        local found = battle.target(snap, query, function(mob)
            local near = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) <= 1
            return near and combat.hits(mob) and snap.self.struck ~= nil
        end)
        pressed[snap] = found ~= nil and (found.choice.value < 0 or found.choice.survives == false) and found or false
    end
    return pressed[snap] or nil
end

local function lifted(snap)
    local best = snap.self.hp
    local heal = recovery.heal(snap)
    if heal ~= nil and heal.mp <= (snap.self.mp or 0) then
        best = math.max(best, math.min(snap.self.max_hp, snap.self.hp + heal.hp))
    end
    for _, dose in ipairs(recovery.doses(snap, "hp")) do
        best = math.max(best, math.min(snap.self.max_hp, snap.self.hp + dose.amount))
    end
    return best
end

local function incoming(snap)
    local total = 0
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and combat.hits(mob) and (mob.speed or 0) > 0 and
            math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
            total = total + combat.blow(snap, mob) / mob.speed
        end
    end
    return total * combat.cast(snap)
end

local function charged(snap)
    local mp, hp = snap.self.mp or 0, snap.self.hp
    local buffs  = combat.buffs(snap)
    local spells = recovery.spells(snap)
    if buffs.charge ~= nil and spells.charge ~= nil and mp >= spells.charge.mp then
        mp, hp = math.max(mp, (snap.self.max_mp or 0) * buffs.charge.chance), recovery.charged(snap)
    end
    for _, dose in ipairs(recovery.doses(snap, "mp")) do
        local after = math.min(snap.self.max_mp or 0, (snap.self.mp or 0) + dose.amount)
        if after > mp then
            mp, hp = after, snap.self.hp
        end
    end
    return mp, hp
end

local function state(snap, hp, mp)
    return setmetatable({ self = setmetatable({ hp = hp, mp = mp }, { __index = snap.self }) }, { __index = snap })
end

local function restored(snap)
    local heal = recovery.heal(snap)
    local hp   = snap.self.hp
    local mp   = snap.self.mp or 0
    while heal ~= nil and heal.hp > 0 and heal.mp > 0 and mp >= heal.mp and hp < snap.self.max_hp do
        hp = math.min(snap.self.max_hp, hp + heal.hp)
        mp = mp - heal.mp
    end
    return hp, mp
end

local function recoverable(snap, query, mob)
    local hp = lifted(snap)
    if hp > snap.self.hp and danger.winnable(state(snap, hp, snap.self.mp or 0), query, mob) then
        return true
    end
    local full, rest = restored(snap)
    if full > hp and danger.winnable(state(snap, full, rest), query, mob) then
        return true
    end
    local mp, left = charged(snap)
    return mp > (snap.self.mp or 0) and danger.winnable(state(snap, left, mp), query, mob)
end

local function planned(snap, query, mob)
    local plan = party.plan(snap, query)
    for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
        if floor.map == snap.world.map and snap.self.hp > (floor.need or 0) then
            for _, name in ipairs(floor.targets or {}) do
                if name == mob.name then
                    return true
                end
            end
        end
    end
    return false
end

local function passing(snap, query, mob)
    local dest = value.objective(snap, query).dest
    if dest == nil or (mob.speed or 0) <= 0 then
        return false
    end
    for _, map in ipairs(dest.maps or { dest.map }) do
        if map == snap.world.map then
            return false
        end
    end
    local window = 2 * data.sight.x * (snap.pace.step or data.walk)
    return snap.self.hp > battle.burst(snap) + combat.blow(snap, mob) / mob.speed * window
end

local function threat(snap, query)
    local fleeing = snap.record.memory.fleeing
    if fleeing ~= nil and snap.now < fleeing.ends and fleeing.map == snap.world.map then
        return { name = fleeing.name }, 1, "FLEEING"
    end
    local fatal = danger.fatal(snap, query)
    if fatal ~= nil and (recoverable(snap, query, fatal) or planned(snap, query, fatal) or passing(snap, query, fatal)) then
        fatal = nil
    end
    if fatal ~= nil then
        return fatal, danger.chance(snap, query, fatal, math.huge), "FATAL"
    end
    local burst = battle.burst(snap)
    local lift = lifted(snap)
    local hurt = incoming(snap)
    if snap.self.struck ~= nil and burst > 0 and snap.self.hp <= burst and
        (lift - hurt <= burst or lift - snap.self.hp <= hurt) then
        for _, mob in ipairs(snap.world.mobs or {}) do
            if mob.x ~= nil and combat.hits(mob) and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
                return mob, 1, "IMMINENT"
            end
        end
    end
    local present = escort.present(snap, query)
    local cohort  = escort.gathering(snap, query)
    local coming  = cohort ~= nil and (present == nil or #cohort.members > #present.members)
    local found   = coming == false and pressing(snap, query) or nil
    local refill  = combat.buffs(snap).charge
    if found ~= nil and refill ~= nil and snap.self.hp > refill.cost + 2 * battle.burst(snap) and
        recoverable(snap, query, found.mob) then
        found = nil
    end
    if found ~= nil then
        return found.mob, (found.choice.plan or {}).p_die or 0, "PRESSING"
    end
    local need = math.max(ground.need(snap, query), battle.burst(snap))
    local crew = escort.present(snap, query)
    local out  = incoming(snap) / combat.cast(snap) * 2 * data.sight.x * (snap.pace.step or data.walk)
    if snap.self.struck ~= nil and snap.self.hp <= math.max(need, out) and recovery.stranded(snap) and
        (crew == nil or crew.heal <= 0) then
        for _, mob in ipairs(snap.world.mobs or {}) do
            if mob.x ~= nil and combat.hits(mob) and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
                return mob, 1, "STRANDED"
            end
        end
    end
    return nil, 0
end

local function rally(snap, query, mob)
    local flow  = incoming(snap) / combat.cast(snap)
    local range = math.min(data.sight.x, data.sight.y)
    local step  = snap.pace.step or data.walk
    local found = nil
    local best  = math.huge
    for _, member in ipairs(party.members(snap)) do
        local world   = member.world
        local staying = member.heading == nil
        for _, map in ipairs(member.heading or {}) do
            staying = staying or map == snap.world.map
        end
        if member.self.ghost == false and world.map == snap.world.map and world.x ~= nil and staying and
            member.task ~= "ESCAPE" then
            local distance = math.abs(world.x - snap.world.x) + math.abs(world.y - snap.world.y)
            local after    = snap.self.hp - flow * math.max(0, distance - range) * step
            if distance > range and distance < best and after > 0 and
                danger.winnable(state(snap, after // 1 | 0, snap.self.mp or 0), query, mob, escort.crew({ member }, query)) then
                found = { map = world.map, x = world.x, y = world.y, range = range }
                best  = distance
            end
        end
    end
    return found
end

local function regroup(snap, query)
    local plan = #(snap.party or {}) > 0 and party.plan(snap, query) or nil
    if plan == nil or plan.gate == nil or plan.gate == snap.world.map then
        return nil
    end
    return { maps = { plan.gate } }
end

function M.wanted(snap, query)
    return snap.self.ghost == false and teleport.returns[snap.self.nation] ~= nil and threat(snap, query) ~= nil
end

function M.propose(snap, query)
    local mob, _, cause = threat(snap, query)
    if data.journal then
        local crew = escort.present(snap, query)
        log("debug",
            string.format(
                "[brain:escape] bot=%s map=%s hp=%s/%s mp=%s cause=%s mob=%s dmg=%s adjacent=%.0f crew=%d party=%d",
                snap.self.name or "", tostring(snap.world.map), tostring(snap.self.hp), tostring(snap.self.max_hp),
                tostring(snap.self.mp), tostring(cause), tostring(mob ~= nil and mob.name),
                tostring(mob ~= nil and mob.damage),
                battle.burst(snap), crew ~= nil and #crew.members or 0, #(snap.party or {})))
    end
    local meet = cause ~= nil and cause ~= "IMMINENT" and cause ~= "FLEEING" and rally(snap, query, mob) or nil
    return {
        kind = ACTION.MOVE,
        cause = cause ~= nil and (cause .. ":" .. tostring(mob.name)) or nil,
        fleeing = meet == nil and cause ~= "FLEEING" and {
            map = snap.world.map,
            name = mob.name,
            ends = (snap.now or 0) + 2 * data.sight.x * (snap.pace.step or data.walk),
        } or nil,
        proven = meet == nil and cause == "FATAL" and
            danger.winnable(score.rested(snap), query, mob) == false,
        dest = meet or regroup(snap, query) or { maps = teleport.returns[snap.self.nation], urgent = true },
        forget = snap.record.death ~= nil
    }
end

function M.value(snap, query)
    local _, chance, cause = threat(snap, query)
    local worth            = value.baseline(snap, query)
    local stay             = chance * danger.loss(snap, query, worth)
    local trip             = value.trip(snap, query, M.propose(snap, query).dest, worth)
    if trip == nil then
        return -math.huge
    end
    return stay - (cause == "STRANDED" and trip.risk or trip.cost)
end

return M
