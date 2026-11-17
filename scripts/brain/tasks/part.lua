local ground = require("brain.policies.ground")
local party  = require("brain.policies.party")
local score  = require("brain.policies.score")

local M = {}

local function waited(snap)
    return math.max(0, snap.now - (snap.record.memory.apart or snap.now)) / 1000
end

local function members(list)
    local found = {}
    for _, name in ipairs(list or {}) do
        found[name] = true
    end
    return found
end

-- A group with no ground to hunt together never walks a lap to end on, so it ends as soon as that is known.
function M.finished(snap, query)
    return party.known(snap) and party.plan(snap, query) == nil
end

-- A group ends only where it can be judged whole: a servicing begun on a finished lap (decide.lap) that this very
-- group walked from the entry it made together (user 11-15). Weighed any time, by the time apart against the time to
-- the next level, a group never ended at 99 and could end mid-hunt below it.
function M.lapped(snap)
    local memory    = snap.record.memory
    local servicing = memory.servicing
    if servicing == nil or servicing.lapped ~= true or memory.joined == nil or servicing.at < memory.joined then
        return false
    end
    local walked = members(memory.joined_crew)
    for _, name in ipairs(servicing.group or {}) do
        if walked[name] == nil then
            return false
        end
    end
    return #(servicing.group or {}) == #(memory.joined_crew or {})
end

-- One that would rather hunt the next lap alone. The same lap on both sides, the rates are compared as they are; a
-- tie keeps the group. A ghost has no hunt to weigh and keeps it.
function M.unwanted(snap, query)
    if snap.self.ghost then
        return false
    end
    local base  = score.recovered(snap, query)
    local plan  = party.plan(base, query)
    local alone = ground.choose(base, query, ground.mode(base))
    local group = plan ~= nil and (plan.top or plan.rate) or 0
    local solo  = alone ~= nil and (alone.top or alone.rate) or 0
    return solo > group
end

-- One of the lap's group is gone: it would not go on, and then the group breaks up at once rather than going on
-- smaller (user 11-15).
function M.dissolved(snap)
    local here = members(snap.group)
    for _, name in ipairs(snap.record.memory.servicing.group or {}) do
        if here[name] == nil then
            return true
        end
    end
    return false
end

function M.propose(snap)
    local names = {}
    for _, profile in ipairs(snap.party or {}) do
        table.insert(names, profile.self.name)
    end
    return { kind = ACTION.LEAVE, names = names, waited = waited(snap) }
end

return M
