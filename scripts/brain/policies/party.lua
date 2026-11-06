local combat = require("brain.data.combat")
local danger = require("brain.policies.danger")
local escort = require("brain.policies.escort")
local ground = require("brain.policies.ground")
local score  = require("brain.policies.score")

local M = {}

local offers  = setmetatable({}, { __mode = "k" })
local members = setmetatable({}, { __mode = "k" })
local plans   = setmetatable({}, { __mode = "k" })

M.member = escort.member

local function summary(plan)
    if plan == nil then
        return nil
    end
    local floors = {}
    for _, floor in ipairs(plan.floors) do
        table.insert(floors, { map = floor.map, targets = floor.targets })
    end
    return {
        name = plan.name,
        rate = plan.rate,
        pain = plan.pain,
        peak = plan.peak,
        floors = floors,
        yields = plan
            .yields,
        gate = plan.gate
    }
end

local function arrival(snap, query, decision)
    if #(snap.group or {}) == 0 or decision == nil or decision.kind ~= ACTION.MOVE or decision.dest == nil then
        return nil
    end
    local route = query.route(decision.dest.maps or { decision.dest.map })
    return route ~= nil and snap.now + route.cost * (snap.pace.step or combat.walk) or nil
end

function M.profile(snap, query, decision)
    local grouped = #(snap.group or {}) > 0
    local base    = score.recovered(snap, query)
    return {
        self    = base.self,
        spells  = snap.spells,
        gear    = base.gear,
        buffs   = snap.buffs,
        bag     = base.bag,
        pace    = snap.pace,
        world   = { map = snap.world.map, x = snap.world.x, y = snap.world.y },
        now     = snap.now,
        grouped = grouped,
        fled    = snap.record.memory.fled,
        task    = decision ~= nil and decision.task or nil,
        heading = decision ~= nil and decision.task == "FIELD" and decision.kind == ACTION.MOVE and decision.dest ~= nil and
            (decision.dest.maps or { decision.dest.map }) or nil,
        arrives = arrival(snap, query, decision),
        battle  = grouped and snap.record.memory.battle or nil,
        marks   = grouped and snap.record.memory.marks or nil,
        plan    = grouped and snap.leader == snap.self.name and summary(M.plan(base, query)) or nil,
    }
end

local function hazards(team, query)
    local found = {}
    for _, member in ipairs(team) do
        for _, id in ipairs(danger.perilous(member, query, false)) do
            found[id] = true
        end
    end
    return found
end

function M.perilous(snap, query)
    local members = M.members(snap)
    if #members == 0 then
        return danger.perilous(snap, query)
    end
    local found = hazards(members, query)
    for _, id in ipairs(danger.perilous(snap, query)) do
        found[id] = true
    end
    local list = {}
    for id in pairs(found) do
        table.insert(list, id)
    end
    table.sort(list)
    return list
end

function M.gate(snap, query, plan, team)
    local first  = plan.floors[1].map
    local others = {}
    for _, member in ipairs(team) do
        if member ~= snap then
            table.insert(others, member)
        end
    end
    local crew  = escort.crew(others, query)
    local route = query.route({ first }, danger.perilous(snap, query, crew), danger.shunned(snap, query))
    if route == nil then
        return nil
    end
    local maps = { snap.world.map }
    for _, hop in ipairs(route.path or {}) do
        table.insert(maps, hop.dest)
    end
    local ids      = hazards(team, query)
    local previous = nil
    for _, map in ipairs(maps) do
        for id in pairs(ids) do
            if query.native(map, id) then
                return previous or map
            end
        end
        previous = map
    end
    return maps[#maps - 1] or first
end

function M.feasible(snap, query, plan, team)
    local gate = M.gate(snap, query, plan, team)
    if gate == nil then
        return nil
    end
    for _, member in ipairs(team) do
        local origin = { map = member.world.map, x = member.world.x, y = member.world.y }
        local route  = member == snap and
            query.route({ gate }, danger.perilous(snap, query, false), danger.shunned(snap, query)) or
            query.path({ maps = { gate } }, danger.perilous(member, query, false), {}, origin)
        if route == nil and member.world.map ~= gate then
            return nil
        end
    end
    return gate
end

local function solo(snap, query)
    local best = ground.choose(snap, query, ground.mode(snap))
    return best ~= nil and best.rate or 0
end

function M.offer(snap, query)
    if snap.self.ghost or snap.self.class == "NONE" or snap.self.grouping ~= true or #(snap.group or {}) > 0 then
        return nil
    end
    if offers[snap] ~= nil then
        return offers[snap] or nil
    end
    local origin = snap
    snap         = score.recovered(snap, query)
    local mine   = solo(snap, query)
    local nearby = {}
    local parted = snap.record.memory.left
    local recent = {}
    if type(parted) == "table" and snap.now - (parted.at or 0) < (parted.waited or 0) * 1000 then
        for _, name in ipairs(parted.names or {}) do
            recent[name] = true
        end
    end
    local crew = snap.record.memory.crew
    for _, entry in ipairs(query.roster()) do
        local profile = entry.profile
        if type(profile) == "table" and recent[entry.name] == nil and profile.grouped == false and
            (crew == nil or crew[entry.name]) and
            profile.self ~= nil and profile.self.grouping == true and profile.self.ghost == false and
            profile.self.class ~= "NONE" then
            table.insert(nearby, entry)
        end
    end
    table.sort(nearby, function(left, right)
        local a = math.abs(left.profile.self.level - snap.self.level)
        local b = math.abs(right.profile.self.level - snap.self.level)
        return a < b or (a == b and left.name < right.name)
    end)
    local best = nil
    for rank, entry in ipairs(nearby) do
        if rank > combat.party.scan then
            break
        end
        local other = M.member(entry.profile)
        local plan  = ground.party({ snap, other }, query, "SUPPLY")
        if plan ~= nil and M.feasible(snap, query, plan, { snap, other }) == nil then
            plan = nil
        end
        if plan ~= nil then
            local gain    = plan.rate - mine
            local partner = plan.rate - solo(other, query)
            if gain > 0 and partner > 0 and (best == nil or gain > best.gain) then
                best = { name = entry.name, gain = gain, plan = plan }
            end
        end
    end
    offers[origin] = best or false
    return best
end

function M.members(snap)
    if members[snap] == nil then
        local found = {}
        for _, profile in ipairs(snap.party or {}) do
            table.insert(found, M.member(profile))
        end
        members[snap] = found
    end
    return members[snap]
end

function M.known(snap)
    return #M.members(snap) >= #(snap.group or {})
end

local held = {}

function M.plan(snap, query)
    local members = M.members(snap)
    if #members == 0 then
        return ground.choose(snap, query)
    end
    if snap.leader ~= nil and snap.leader ~= snap.self.name then
        for _, member in ipairs(members) do
            if member.self.name == snap.leader and member.plan ~= nil then
                return member.plan
            end
        end
    end
    if plans[snap] == nil then
        local team = { snap }
        for _, member in ipairs(members) do
            table.insert(team, member)
        end
        local commit = snap.record.memory.commit
        local parts  = { (snap.now or 0) // combat.rerank, tostring(commit), table.concat(danger.shunned(snap, query),
            ",") }
        for _, member in ipairs(team) do
            table.insert(parts, score.signature(member, ""))
            table.insert(parts, tostring(member.self.ghost))
        end
        local key  = table.concat(parts, "|")
        local kept = held[snap.self.name]
        local plan = nil
        if kept ~= nil and kept.key == key then
            plan = kept.plan
        else
            plan = commit ~= nil and ground.hold(team, query, commit) or nil
            plan = plan or ground.party(team, query, "SUPPLY")
            held[snap.self.name] = { key = key, plan = plan }
        end
        if plan ~= nil then
            local copy = {}
            for key, entry in pairs(plan) do
                copy[key] = entry
            end
            copy.gate = M.gate(snap, query, plan, team) or plan.floors[1].map
            plan      = copy
        end
        plans[snap] = plan or false
    end
    return plans[snap] or nil
end

function M.leader(snap)
    for _, member in ipairs(M.members(snap)) do
        if member.self.name == snap.leader then
            return member
        end
    end
    return nil
end

return M
