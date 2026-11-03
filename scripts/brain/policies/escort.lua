local combat   = require("brain.data.combat")
local magic    = require("brain.data.spells")
local fight    = require("brain.policies.combat")
local score    = require("brain.policies.score")
local survival = require("brain.policies.survival")

local M = {}

local presents = setmetatable({}, { __mode = "k" })

function M.member(profile)
    return {
        self   = profile.self,
        spells = profile.spells or {},
        gear   = profile.gear or {},
        buffs  = profile.buffs or {},
        bag    = profile.bag or { items = {}, used = 0, capacity = 0 },
        pace   = profile.pace or {},
        world  = { map = profile.world.map, x = profile.world.x, y = profile.world.y, mobs = {}, items = {} },
        record = { failures = {}, memory = {} },
        now    = profile.now,
        plan   = profile.plan,
        battle = profile.battle,
    }
end

function M.crew(members, query)
    local keys  = {}
    local heals = {}
    local heal  = 0
    local bonus = 0
    for index, member in ipairs(members) do
        table.insert(keys, score.signature(member, ""))
        heals[index] = score.capability(member, query).hps.others
        heal         = heal + heals[index]
        bonus        = math.max(bonus, combat.group.bonus[member.self.class] or 0)
    end
    local assist = {}
    for _, member in ipairs(members) do
        for _, learned in ipairs(member.spells or {}) do
            local data = magic.spells[learned.name]
            if data ~= nil and data.role == "DEBUFF" and type(data.mp or 0) == "number" and
                (data.mp or 0) <= (member.self.mp or 0) then
                assist[learned.name] = true
            end
        end
    end
    return {
        members = members,
        assist  = assist,
        size    = #members + 1,
        heals   = heals,
        heal    = heal,
        support = heal,
        bonus   = #members > 0 and bonus or 0,
        key     = "|crew:" .. table.concat(keys, ";"),
    }
end

local function near(snap, member)
    local world = member.world
    return member.self ~= nil and member.self.ghost == false and world.map == snap.world.map and world.x ~= nil and
        math.abs(world.x - snap.world.x) <= combat.sight.x and math.abs(world.y - snap.world.y) <= combat.sight.y
end

function M.present(snap, query)
    if presents[snap] == nil then
        local found = {}
        for _, profile in ipairs(snap.party or {}) do
            local member = M.member(profile)
            if near(snap, member) then
                table.insert(found, member)
            end
        end
        presents[snap] = #found > 0 and M.crew(found, query) or false
    end
    return presents[snap] or nil
end

function M.complete(snap, query)
    local crew = M.present(snap, query)
    return #(snap.party or {}) > 0 and crew ~= nil and #crew.members == #snap.party
end

local function arrange(snap, query, mob, crew, density, own, share)
    local support = share * crew.heal
    local aid     = (support + own) / 1000
    local speed   = 0
    for index, member in ipairs(crew.members) do
        local helper = fight.outcome(member, query, mob, density, aid)
        if helper.time < math.huge then
            local busy = crew.heals[index] > 0 and share or 0
            speed      = speed + (1 - busy) / helper.time
        end
    end
    return { density = density, aid = aid, speed = speed, support = support, share = share, assist = crew.assist }
end

local function clamp(value)
    return math.max(0, math.min(1, value))
end

function M.context(snap, query, mob, crew, density, own)
    own = own or 0
    if crew == nil then
        return { density = density, aid = own > 0 and own / 1000 or nil }
    end
    if crew.heal <= 0 then
        return arrange(snap, query, mob, crew, density, own, 0)
    end
    local shares = { 1 }
    local idle   = arrange(snap, query, mob, crew, density, own, 0)
    local alone  = survival.fight(snap, query, mob, idle)
    if alone.time < math.huge and alone.time > 0 then
        table.insert(shares, clamp((alone.taken - alone.healed) / alone.time * 1000 / crew.heal))
    end
    local opener = fight.opener(snap, mob)
    if opener ~= nil and opener.hp > 0 then
        table.insert(shares, clamp((opener.hp * 1000 / fight.cast(snap) - own) / crew.heal))
    end
    local best   = nil
    local result = nil
    for _, share in ipairs(shares) do
        local context = arrange(snap, query, mob, crew, density, own, share)
        local fought  = survival.fight(snap, query, mob, context)
        local better  = result == nil or
            (fought.ending > 0 and (result.ending <= 0 or fought.time < result.time)) or
            (fought.ending <= 0 and result.ending <= 0 and fought.ending > result.ending)
        if better then
            best   = context
            result = fought
        end
    end
    return best
end

return M
