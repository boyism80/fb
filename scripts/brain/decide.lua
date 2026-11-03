local alert    = require("brain.policies.alert")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local failure  = require("brain.policies.failure")
local party    = require("brain.policies.party")
local states   = require("brain.data.states")
local profile  = require("brain.profile")
local query    = require("brain.query")
local snapshot = require("brain.snapshot")

local M = {}

local stalls = {}

local function module(name)
    return require("brain.tasks." .. string.lower(name))
end

local function decision(snap, facts, state, name, task, revisit)
    local result = task.propose ~= nil and
        profile.measure(snap.self.name, "propose:" .. name, task.propose, snap, facts) or
        { kind = ACTION.TASK }
    if combat.casting(result) and combat.cooldown(snap) > 0 then
        result = { kind = ACTION.HOLD }
    end
    result.state   = state
    result.task    = name
    result.step    = task.step ~= nil and task.step(snap) or 0
    result.reason  = name
    result.revisit = revisit
    return result
end

local function soonest(revisit, value)
    if value == nil then
        return revisit
    end
    return math.min(revisit or value, value)
end

function M.choose(snap, facts, current)
    local revisit = nil
    local passed  = false
    local best    = nil
    local held    = nil
    local values  = {}
    local blocked = {}
    for _, state in ipairs(states) do
        for _, name in ipairs(state.tasks) do
            local task = module(name)
            if profile.measure(snap.self.name, "wanted:" .. name, task.wanted, snap, facts) then
                local remaining = failure.remaining(snap, name)
                if remaining == 0 and states.required[name] then
                    if name == current or (passed and current ~= nil and states.required[current]) then
                        return nil, revisit
                    end
                    return decision(snap, facts, state.name, name, task, revisit), revisit
                elseif remaining == 0 then
                    local delta = task.value ~= nil and
                        profile.measure(snap.self.name, "value:" .. name, task.value, snap,
                            facts) or 0
                    table.insert(values, string.format("%s=%.1f", name, delta))
                    if name == current then
                        held = delta
                    elseif best == nil or delta > best.delta then
                        best = { state = state.name, name = name, task = task, delta = delta }
                    end
                else
                    revisit = soonest(revisit, remaining)
                    table.insert(blocked, string.format("%s=%s", name, remaining and
                        string.format("%.1fs", remaining / 1000) or "barred"))
                end
            elseif task.revisit ~= nil then
                revisit = soonest(revisit, task.revisit(snap, facts))
            end
            passed = passed or name == current
        end
    end
    local owner   = snap.self.name or ""
    local stalled = best == nil and current == nil and #blocked > 0
    if stalled and stalls[owner] == nil then
        log("warn", string.format("[brain:anomaly] bot=%s stalled: every wanted task is backing off %s", owner,
            table.concat(blocked, " ")))
    end
    stalls[owner] = stalled or nil
    if best ~= nil and best.delta < 0 and current == nil then
        log("debug", string.format("[brain:decide] %s picks nothing", table.concat(values, " ")))
    end
    if best == nil or best.delta < 0 or (held ~= nil and best.delta <= held) or
        (current ~= nil and states.required[current]) then
        return nil, revisit
    end
    if #values > 1 then
        log("debug", string.format("[brain:decide] %s picks %s", table.concat(values, " "), best.name))
    end
    return decision(snap, facts, best.state, best.name, best.task, revisit), revisit
end

function M.first(brain, bot, current)
    local owner = bot:name()
    local snap  = profile.measure(owner, "snapshot", snapshot.take, brain, bot)
    local facts = query.bind(brain, bot)
    brain:post(bot, party.profile(snap, facts))
    local apart = #(snap.party or {}) > 0 and escort.complete(snap, facts) == false
    if apart and snap.record.memory.apart == nil then
        brain:remember(bot, "apart", snap.now)
        snap.record.memory.apart = snap.now
    elseif apart == false and snap.record.memory.apart ~= nil then
        brain:remember(bot, "apart", nil)
        snap.record.memory.apart = nil
    end
    local threat = snap.self.ghost == false and profile.measure(owner, "fatal", danger.fatal, snap, facts) or nil
    if threat ~= nil then
        log("debug", string.format("[brain:decide] peril %s attack=%s at map=%d %d,%d", tostring(threat.name),
            tostring(threat.attack), snap.world.map, threat.x or -1, threat.y or -1))
        local perils = snap.record.memory.perils or {}
        perils[tostring(snap.world.map)] = { mob = threat.id, stats = threat, at = snap.now }
        brain:remember(bot, "perils", perils)
        snap.record.memory.perils = perils
    end
    profile.measure(owner, "alert", alert.watch, brain, bot, snap, facts)
    return M.choose(snap, facts, current)
end

function M.decide(brain, bot)
    profile.start(bot:name())
    local decision, revisit = M.first(brain, bot, nil)
    profile.finish(brain, bot, bot:name())
    return decision or { state = "PREPARE", reason = "idle", kind = ACTION.HOLD, revisit = revisit }
end

return M
