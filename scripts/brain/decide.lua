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
local value    = require("brain.policies.value")

local M = {}

local stalls = {}
local picks  = {}

local IDLE = { "AID", "REST", "LOOT" }
local ERRANDS = { SUPPLY = true, SELL = true, OUTFIT = true, LEARN = true }

local function module(name)
    return require("brain.tasks." .. string.lower(name))
end

local function idle(snap, facts, casting)
    local found, spare, most = nil, nil, 0
    for _, name in ipairs(IDLE) do
        local task = module(name)
        if failure.remaining(snap, name) == 0 and task.wanted(snap, facts) then
            local worth = task.value(snap, facts)
            if worth > most then
                local action = task.propose(snap, facts)
                if (action.kind ~= ACTION.MOVE or name == "LOOT") and (casting == false or combat.casting(action) == false) then
                    found, spare, most = action, name, worth
                end
            end
        end
    end
    return found, spare
end

local function decision(snap, facts, state, name, task, revisit)
    local result   = task.propose ~= nil and
        profile.measure(snap.self.name, "propose:" .. name, task.propose, snap, facts) or
        { kind = ACTION.TASK }
    local casting  = combat.casting(result)
    local cooldown = casting and combat.cooldown(snap) or
        (combat.swinging(snap, result) and combat.backswing(snap)) or 0
    if cooldown > 0 then
        local action, spare = idle(snap, facts, casting)
        if action ~= nil then
            result, name, task = action, spare, module(spare)
        else
            result  = { kind = ACTION.HOLD }
            revisit = math.min(revisit or cooldown, cooldown)
        end
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
                "[brain:act] bot=%s class=%s t=%s map=%s task=%s kind=%s key=%s item=%s spell=%s mob=%s hp=%s/%s mp=%s/%s exp=%s seen=%d close=%d far=%d lv=%s partner=%s text=%s",
                snap.self.name or "", tostring(snap.self.class), tostring(snap.now), tostring(snap.world.map), name,
                tostring(result.kind), tostring(result.key), tostring(result.item), tostring(spell), tostring(result.mob),
                tostring(snap.self.hp), tostring(snap.self.max_hp), tostring(snap.self.mp), tostring(snap.self.max_mp),
                tostring(snap.self.exp), seen, close, far, tostring(snap.self.level), partner, tostring(result.text)))
    end
    result.state   = state
    result.task    = name
    result.step    = task.step ~= nil and task.step(snap) or 0
    result.reason  = name
    result.revisit = revisit
    return result
end

local journals = {}
local hops     = {}

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
                    if ERRANDS[name] and delta > -math.huge and delta < math.huge then
                        delta = delta - value.deserted(snap, facts, task.propose(snap, facts))
                    end
                    local errand = snap.record.memory.errand
                    if errand ~= nil and errand.name == name and delta > -math.huge and delta < math.huge then
                        delta = delta + math.max(0, snap.now - errand.at) / 1000 * value.baseline(snap, facts).value
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
    if data.journal and (numbers.DEFEND ~= nil or numbers.AID ~= nil or numbers.REST ~= nil) and #party.members(snap) > 0 then
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
    if data.journal and journals[owner] ~= best.name then
        journals[owner] = best.name
        local parts = {}
        for key, number in pairs(numbers) do
            table.insert(parts, string.format("%s=%.0f", key, number))
        end
        table.sort(parts)
        log("debug",
            string.format("[brain:pick] bot=%s t=%s map=%s hp=%s mp=%s money=%s pick=%s %s", owner, tostring(snap.now),
                tostring(snap.world.map), tostring(snap.self.hp), tostring(snap.self.mp), tostring(snap.self.money),
                best.name,
                table.concat(parts, " ")))
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

function M.mark(snap, decision)
    local marks = snap.record.memory.marks
    if marks == nil and (decision == nil or decision.debuff == nil) then
        return false
    end
    local alive = {}
    for _, mob in ipairs(snap.world.mobs or {}) do
        alive[tostring(mob.oid)] = true
    end
    local kept    = {}
    local changed = false
    for key, names in pairs(marks or {}) do
        if alive[key] then
            kept[key] = names
        else
            changed = true
        end
    end
    if decision ~= nil and decision.debuff ~= nil and decision.oid ~= nil then
        local key   = tostring(decision.oid)
        local names = {}
        for name, at in pairs(kept[key] or {}) do
            names[name] = at
        end
        names[decision.debuff] = snap.now
        kept[key]              = names
        changed                = true
    end
    if changed == false then
        return false
    end
    snap.record.memory.marks = next(kept) ~= nil and kept or nil
    return true, snap.record.memory.marks
end

function M.errand(snap, decision)
    local errand = snap.record.memory.errand
    local going  = decision ~= nil and ERRANDS[decision.task] and
        (decision.kind == ACTION.MOVE or decision.kind == ACTION.SAY or decision.kind == ACTION.TALK)
    if going and errand ~= nil and errand.name == decision.task then
        return false
    end
    local found = going and { name = decision.task, at = snap.now } or nil
    if found == nil and errand == nil then
        return false
    end
    snap.record.memory.errand = found
    return true, found
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
    for _, member in ipairs(decision.hemmed and {} or party.members(snap)) do
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
    if data.journal then
        local before = hops[owner]
        if before ~= nil and before.map ~= snap.world.map then
            log("debug",
                string.format("[brain:hop] bot=%s t=%s gap=%.0f from=%s:%s,%s to=%s:%s,%s task=%s kind=%s dest=%s", owner,
                    tostring(snap.now), snap.now - before.t,
                    tostring(before.map), tostring(before.x), tostring(before.y), tostring(snap.world.map),
                    tostring(snap.world.x), tostring(snap.world.y),
                    tostring(before.task), tostring(before.kind), tostring(before.dest)))
        end
        local dest = decision ~= nil and decision.dest or nil
        hops[owner] = {
            map = snap.world.map,
            x = snap.world.x,
            y = snap.world.y,
            t = snap.now,
            task = decision ~= nil and decision.task or nil,
            kind = decision ~= nil and decision.kind or nil,
            dest = dest ~= nil and (dest.map or (dest.maps ~= nil and dest.maps[1])) or nil
        }
    end
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
    local changed, errand = M.errand(snap, decision)
    if changed then
        brain:remember(bot, "errand", errand)
    end
    local remarked, marks = M.mark(snap, decision)
    if remarked then
        brain:remember(bot, "marks", marks)
    end
    if decision ~= nil and decision.leg ~= nil then
        snap.record.memory.leg = decision.leg
        brain:remember(bot, "leg", decision.leg)
    end
    if decision ~= nil and decision.aim ~= nil and decision.aim ~= snap.record.memory.aim then
        snap.record.memory.aim = decision.aim
        brain:remember(bot, "aim", decision.aim)
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
