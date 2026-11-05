local data     = require("brain.data.combat")
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
local trace    = require("brain.trace")

local M = {}

local stalls = {}
local picks  = {}

local function module(name)
    return require("brain.tasks." .. string.lower(name))
end

local function decision(snap, facts, state, name, task, revisit)
    local result = task.propose ~= nil and
        profile.measure(snap.self.name, "propose:" .. name, task.propose, snap, facts) or
        { kind = ACTION.TASK }
    local cooldown = combat.casting(result) and combat.cooldown(snap) or
        (combat.swinging(snap, result) and combat.backswing(snap)) or 0
    if cooldown > 0 then
        result  = { kind = ACTION.HOLD }
        revisit = math.min(revisit or cooldown, cooldown)
    end
    if data.journal then
        local spell = nil
        for _, learned in ipairs(snap.spells or {}) do
            local slot = result.slot or (result.maneuver ~= nil and result.maneuver.slot) or nil
            if slot ~= nil and learned.slot == slot then
                spell = learned.name
            end
        end
        local seen, close, far = 0, 0, 0
        for _, mob in ipairs(snap.world.mobs or {}) do
            if mob.x ~= nil then
                seen = seen + 1
                if math.abs(mob.x - snap.world.x) > data.sight.x or math.abs(mob.y - snap.world.y) > data.sight.y then
                    far = far + 1
                end
                if math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
                    close = close + 1
                end
            end
        end
        local partner = "-"
        for _, member in ipairs(party.members(snap)) do
            local ally = facts.ally(member.self.name)
            partner = string.format("%s:%s:%s:%s", tostring(member.world.map),
                member.world.x ~= nil and
                math.abs(member.world.x - snap.world.x) + math.abs(member.world.y - snap.world.y) or "?",
                ally ~= nil and tostring(ally.percent) or "unseen", tostring(member.self.hp))
        end
        log("debug",
            string.format(
                "[brain:act] bot=%s class=%s t=%s map=%s task=%s kind=%s key=%s item=%s spell=%s mob=%s hp=%s/%s mp=%s/%s exp=%s seen=%d close=%d far=%d lv=%s partner=%s",
                snap.self.name or "", tostring(snap.self.class), tostring(snap.now), tostring(snap.world.map), name,
                tostring(result.kind), tostring(result.key), tostring(result.item), tostring(spell), tostring(result.mob),
                tostring(snap.self.hp), tostring(snap.self.max_hp), tostring(snap.self.mp), tostring(snap.self.max_mp),
                tostring(snap.self.exp), seen, close, far, tostring(snap.self.level), partner))
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

function M.choose(snap, facts, current, sink)
    local revisit = nil
    local passed  = false
    local best    = nil
    local held    = nil
    local values  = {}
    local numbers = {}
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
                    if data.trace then
                        table.insert(values, string.format("%s=%.1f", name, delta))
                    end
                    numbers[name] = delta < math.huge and delta > -math.huge and delta or (delta > 0 and 1e300 or -1e300)
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
    if data.journal and numbers.DEFEND ~= nil and #party.members(snap) > 0 then
        log("debug", string.format("[brain:values] class=%s t=%s hp=%s mp=%s AID=%s DEFEND=%s REST=%s ESCAPE=%s",
            tostring(snap.self.class), tostring(snap.now), tostring(snap.self.hp), tostring(snap.self.mp),
            tostring(numbers.AID), tostring(numbers.DEFEND), tostring(numbers.REST), tostring(numbers.ESCAPE)))
    end
    local owner   = snap.self.name or ""
    local stalled = best == nil and current == nil and #blocked > 0
    if stalled and stalls[owner] == nil then
        log("warn", string.format("[brain:anomaly] bot=%s stalled: every wanted task is backing off %s", owner,
            table.concat(blocked, " ")))
    end
    stalls[owner] = stalled or nil
    if data.trace and best ~= nil and best.delta < 0 and current == nil then
        log("debug", string.format("[brain:decide] %s picks nothing", table.concat(values, " ")))
    end
    if best == nil or best.delta < 0 or (held ~= nil and best.delta <= held) or
        (current ~= nil and states.required[current]) then
        return nil, revisit
    end
    if data.trace and #values > 1 then
        log("debug", string.format("[brain:decide] %s picks %s", table.concat(values, " "), best.name))
    end
    if sink ~= nil and picks[owner] ~= best.name then
        picks[owner] = best.name
        trace.decide(sink.brain, sink.bot, snap, numbers, best.name, current)
    end
    return decision(snap, facts, best.state, best.name, best.task, revisit), revisit
end

function M.swung(snap, decision)
    if decision == nil or combat.swinging(snap, decision) == false then
        return nil
    end
    snap.record.memory.swung = snap.now
    return snap.now
end

function M.fleeing(snap, decision)
    if decision == nil or decision.fleeing == nil then
        return nil
    end
    snap.record.memory.fleeing = decision.fleeing
    return decision.fleeing
end

function M.warded(snap, decision)
    if decision == nil or decision.ward == nil then
        return nil
    end
    local warded = snap.record.memory.warded or {}
    warded[decision.ward] = snap.now
    snap.record.memory.warded = warded
    return warded
end

function M.flee(snap, decision)
    if decision == nil or decision.task ~= "ESCAPE" or snap.self.ghost or danger.floor(snap.world.map) == false then
        return nil
    end
    if decision.proven == false then
        return nil
    end
    for _, member in ipairs(party.members(snap)) do
        if member.world.map ~= snap.world.map then
            return nil
        end
    end
    local fled = snap.record.memory.fled or {}
    local key  = tostring(snap.world.map)
    local mine = danger.power(snap)
    if type(fled[key]) == "table" and fled[key].power == mine then
        return nil
    end
    fled[key] = { power = mine, at = snap.now }
    snap.record.memory.fled = fled
    return fled
end

function M.commit(snap, facts)
    local memory = snap.record.memory
    if #(snap.group or {}) == 0 then
        return memory.commit ~= nil or memory.swept ~= nil, nil, nil
    end
    if party.known(snap) == false then
        return false, memory.commit, memory.swept
    end
    local plan  = party.plan(snap, facts)
    local name  = snap.leader == snap.self.name and (plan ~= nil and plan.name or nil) or memory.commit
    local swept = memory.swept
    if plan ~= nil and (swept == nil or swept.name ~= plan.name) then
        swept = { name = plan.name, floors = {} }
    end
    return name ~= memory.commit or swept ~= memory.swept, name, swept
end

function M.sweep(snap, decision)
    local swept = snap.record.memory.swept
    if swept == nil or decision == nil or decision.kind ~= ACTION.ATTACK or
        swept.floors[tostring(snap.world.map)] ~= nil or decision.occupy == nil or decision.occupy.map ~= snap.world.map then
        return nil
    end
    local copy = { name = swept.name, floors = {} }
    for key, value in pairs(swept.floors) do
        copy.floors[key] = value
    end
    copy.floors[tostring(snap.world.map)] = true
    return copy
end

function M.first(brain, bot, current)
    local owner = bot:name()
    local snap  = profile.measure(owner, "snapshot", snapshot.take, brain, bot)
    local facts = query.bind(brain, bot)
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
    local changed, commit, swept = M.commit(snap, facts)
    if changed then
        brain:remember(bot, "commit", commit)
        brain:remember(bot, "swept", swept)
        snap.record.memory.commit = commit
        snap.record.memory.swept  = swept
    end
    local decision, revisit = M.choose(snap, facts, current, { brain = brain, bot = bot })
    brain:post(bot, party.profile(snap, facts, decision))
    local fled = M.flee(snap, decision)
    if fled ~= nil then
        brain:remember(bot, "fled", fled)
    end
    local swept = M.sweep(snap, decision)
    if swept ~= nil then
        brain:remember(bot, "swept", swept)
    end
    local warded = M.warded(snap, decision)
    if warded ~= nil then
        brain:remember(bot, "warded", warded)
    end
    local fleeing = M.fleeing(snap, decision)
    if fleeing ~= nil then
        brain:remember(bot, "fleeing", fleeing)
    end
    local swung = M.swung(snap, decision)
    if swung ~= nil then
        brain:remember(bot, "swung", swung)
    end
    return decision, revisit
end

function M.decide(brain, bot)
    profile.start(bot:name())
    local decision, revisit = M.first(brain, bot, nil)
    profile.finish(brain, bot, bot:name())
    return decision or { state = "PREPARE", reason = "idle", kind = ACTION.HOLD, revisit = revisit }
end

return M
