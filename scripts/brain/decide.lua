local data     = require("brain.data.combat")
local alert    = require("brain.policies.alert")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local failure  = require("brain.policies.failure")
local ground   = require("brain.policies.ground")
local hunting  = require("brain.data.hunting")
local party    = require("brain.policies.party")
local states   = require("brain.data.states")
local profile  = require("brain.profile")
local query    = require("brain.query")
local snapshot = require("brain.snapshot")
local trace    = require("brain.trace")
local value    = require("brain.policies.value")
local tree     = require("brain.tree")
local main     = require("brain.trees.main")
local spare    = require("brain.trees.idle")
local together = require("brain.trees.together")

local M = {}

M.offers = {}

local picks = {}

local ERRANDS = { SUPPLY = true, SELL = true, OUTFIT = true, LEARN = true }

local function module(name)
    return require("brain.tasks." .. string.lower(name))
end

local function idle(snap, facts, casting, gap)
    local found  = tree.run(spare, snap, facts)
    local action = found ~= nil and (found.action or found.task.propose(snap, facts)) or nil
    if action ~= nil and (action.kind ~= ACTION.MOVE or found.name == "LOOT") and
        (casting == false or combat.casting(action) == false) then
        return action, found.name
    end
    local closer = together.closer(snap, gap)
    if closer ~= nil then
        return closer, "FIELD"
    end
    return nil, nil
end

local function describe(action)
    if action == nil then
        return "-"
    end
    local dest = action.dest
    return tostring(action.key or action.item or action.text or action.buff or
        (dest ~= nil and (dest.map or (dest.maps ~= nil and dest.maps[1]))) or action.kind)
end

local function decision(snap, facts, found, revisit)
    local state, name, task = found.state, found.name, found.task
    local result            = found.action or (task.propose ~= nil and
            profile.measure(snap.self.name, "propose:" .. name, task.propose, snap, facts)) or
        { kind = ACTION.TASK }
    if result.kind == ACTION.MOVE and result.dest ~= nil and value.recoiling(snap, facts, result.dest) then
        revisit = snap.record.memory.fleeing.ends - snap.now
        result  = { kind = ACTION.HOLD }
    end
    local casting  = combat.casting(result)
    local cooldown = casting and combat.cooldown(snap) or
        (combat.swinging(snap, result) and combat.backswing(snap)) or 0
    if cooldown > 0 then
        local action, spare = idle(snap, facts, casting, cooldown)
        if action ~= nil then
            result, name, task = action, spare, module(spare)
        else
            result  = { kind = ACTION.HOLD, pause = cooldown }
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
    -- The leader's walk on a hunting floor keeps the crew within reach: the walker stops (LEASHED) before
    -- a step that leaves the sight box of a crewmate who is beside it now, and the group decides again. A
    -- leader that walked the long way round a wall left its poet behind and died alone 15 tiles away
    -- (WA+PO 세작, 534-546s). Only the leader is leashed: it sets the direction and the crew follows it;
    -- leashing everyone made each wait for the other to move. A crewmate that cannot move or is out of
    -- sight already is not leashed: the rally fetches it, and walking back to it is never held back.
    if result.kind == ACTION.MOVE and result.dest ~= nil and result.dest.x ~= nil and result.dest.map == snap.world.map and
        name ~= "ESCAPE" and snap.leader == snap.self.name and ground.floor_at(snap.world.map) ~= nil then
        local names = {}
        for _, member in ipairs(party.members(snap)) do
            local ally = member.self.ghost == false and member.pinned ~= true and facts.ally(member.self.name) or nil
            if ally ~= nil and ally.x ~= nil and math.abs(ally.x - snap.world.x) <= data.sight.x and
                math.abs(ally.y - snap.world.y) <= data.sight.y then
                table.insert(names, member.self.name)
            end
        end
        if #names > 0 then
            local dest = {}
            for key, entry in pairs(result.dest) do
                dest[key] = entry
            end
            dest.leash  = { names = names, x = data.sight.x, y = data.sight.y }
            result.dest = dest
        end
    end
    result.rivals = nil
    result.scale  = nil
    result.state  = state
    result.task   = name
    result.step   = task.step ~= nil and task.step(snap) or 0
    result.reason = name
    if result.revisit ~= nil and revisit ~= nil then
        result.revisit = math.min(result.revisit, revisit)
    elseif result.revisit == nil then
        result.revisit = revisit
    end
    return result
end

local hops = {}

function M.choose(snap, facts, current, sink)
    local found, path = tree.run(main, snap, facts)
    local revisit     = snap.revisit
    local owner       = snap.self.name or ""
    if found == nil then
        return nil, revisit
    end
    if current ~= nil and (found.name == current or states.required[current]) then
        return nil, revisit
    end
    if sink ~= nil and picks[owner] ~= found.name then
        picks[owner] = found.name
        trace.decide(sink.brain, sink.bot, snap, {}, found.name, current)
    end
    local result = decision(snap, facts, found, revisit)
    if data.journal or data.trace then
        M.offers[owner] = { {
            task = result.task,
            value = found.worth or 0,
            action = describe(result),
            path = table.concat(path, ">") .. (found.via ~= nil and ">" .. found.via or "")
        } }
    end
    return result, revisit
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

function M.entered(snap, facts)
    local roster = ground.roster_of(snap.world.map)
    local entry  = snap.record.memory.entry
    if snap.self.ghost or (roster == nil and entry == nil) or (entry ~= nil and entry.roster == roster) then
        return false, entry
    end
    if roster == nil then
        return true, nil
    end
    if entry ~= nil then
        return true, { roster = roster, ready = entry.ready, at = entry.at }
    end
    local rest  = require("brain.tasks.rest")
    local ready = rest.low(snap, facts) == false and (snap.self.mp or 0) >= rest.mana_need(snap, facts)
    return true, { roster = roster, ready = ready, at = snap.now }
end

function M.flee(snap, decision)
    if decision == nil or decision.task ~= "ESCAPE" or snap.self.ghost or
        (danger.floor(snap.world.map) == false and decision.hemmed ~= true) then
        return nil
    end
    for _, member in ipairs(decision.hemmed and {} or party.members(snap)) do
        if member.world.map ~= snap.world.map then
            return nil
        end
    end
    local fled   = snap.record.memory.fled or {}
    local mine   = danger.power(snap)
    local roster = ground.roster_of(snap.world.map)
    local marked = false
    if decision.proven ~= false and danger.fled(snap, snap.world.map) == false then
        fled[tostring(snap.world.map)] = { power = mine, at = snap.now, ends = snap.now + (decision.revive or 0) }
        marked = true
    end
    if roster ~= nil then
        local prior   = fled["r:" .. roster]
        local strikes = type(prior) == "table" and prior.power == mine and prior.strikes or 0
        local entry   = snap.record.memory.entry
        if type(entry) == "table" and entry.roster == roster and entry.ready then
            strikes = strikes + 1
        end
        fled["r:" .. roster] = { power = mine, at = snap.now, strikes = strikes, map = snap.world.map }
        marked = true
    end
    if marked == false then
        return nil
    end
    snap.record.memory.fled = fled
    local plan              = snap.record.memory.commit or snap.record.memory.solo
    local released          = false
    for _, bundle in ipairs(roster ~= nil and hunting.bundles or {}) do
        for _, floor in ipairs(bundle.name == plan and bundle.floors or {}) do
            released = released or floor.map == snap.world.map
        end
    end
    return fled, released
end

function M.commit(snap, facts)
    local memory = snap.record.memory
    local alone = #(snap.group or {}) == 0
    if alone == false and party.known(snap) == false then
        return false, memory.commit, memory.swept, memory.solo
    end
    local plan   = party.plan(snap, facts)
    local chosen = plan ~= nil and plan.name or nil
    local name   = nil
    local solo   = nil
    if alone then
        solo = chosen
    else
        name = snap.leader == snap.self.name and chosen or memory.commit
    end
    local swept = memory.swept
    if plan ~= nil and (swept == nil or swept.name ~= plan.name) then
        swept = { name = plan.name, floors = {} }
    end
    return name ~= memory.commit or swept ~= memory.swept or solo ~= memory.solo, name, swept, solo
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

function M.lap(brain, bot, snap, decision, facts)
    local memory = snap.record.memory
    if decision == nil then
        return
    end
    if memory.servicing == nil and memory.serviced == nil then
        memory.servicing = { at = snap.now }
        brain:remember(bot, "servicing", memory.servicing)
        return
    end
    if decision.lap_done ~= nil then
        local dry = {}
        for map, at in pairs(memory.dry or {}) do
            dry[map] = at
        end
        for _, map in ipairs(decision.lap_done.maps) do
            dry[tostring(map)] = snap.now
        end
        memory.dry       = dry
        memory.servicing = { at = snap.now, leave = true }
        memory.commit    = nil
        memory.solo      = nil
        memory.swept     = nil
        memory.leg       = nil
        memory.aim       = nil
        memory.search    = nil
        memory.roam      = nil
        brain:remember(bot, "roam", nil)
        brain:remember(bot, "dry", dry)
        brain:remember(bot, "servicing", memory.servicing)
        brain:remember(bot, "commit", nil)
        brain:remember(bot, "solo", nil)
        brain:remember(bot, "swept", nil)
        brain:remember(bot, "leg", nil)
        brain:remember(bot, "aim", nil)
        brain:remember(bot, "search", nil)
        return
    end
    if decision.searching then
        if memory.search == nil or memory.search.map ~= snap.world.map then
            memory.search = { map = snap.world.map, from = snap.now }
            brain:remember(bot, "search", memory.search)
        end
    elseif decision.kind == ACTION.ATTACK then
        if memory.search ~= nil then
            memory.search = nil
            brain:remember(bot, "search", nil)
        end
    end
    if decision.viewed ~= nil then
        local roam = { map = snap.world.map, keys = {} }
        if memory.roam ~= nil and memory.roam.map == snap.world.map then
            for key in pairs(memory.roam.keys) do
                roam.keys[key] = true
            end
        end
        for _, key in ipairs(decision.viewed) do
            roam.keys[key] = true
        end
        memory.roam = roam
        brain:remember(bot, "roam", roam)
    end
    -- Deciding to service is shared by the whole group: whoever starts it (the bundle ended, or a member
    -- leaves the ground mid-hunt for supplies), everyone leaves the ground, services on its own and meets
    -- at the entrance again. Before, only the leader's servicing reached the others, and a member leaving
    -- for supplies started none, so the rest kept hunting alone on the floor (WA+PO 세작, 656s).
    local members = party.members(snap)
    if #members > 0 and memory.servicing == nil and party.errands[decision.task] and
        decision.kind == ACTION.MOVE and ground.floor_at(snap.world.map) ~= nil then
        memory.servicing = { at = snap.now, leave = true }
        brain:remember(bot, "servicing", memory.servicing)
    end
    local newest = nil
    for _, member in ipairs(members) do
        local started = member.servicing
        if started ~= nil and started.at ~= memory.serviced and (newest == nil or started.at > newest.at) and
            (memory.servicing == nil or started.at > memory.servicing.at) then
            newest = started
        end
    end
    if newest ~= nil then
        memory.servicing = newest
        memory.serviced  = newest.at
        brain:remember(bot, "servicing", memory.servicing)
        brain:remember(bot, "serviced", memory.serviced)
    end
    if memory.servicing ~= nil and decision.task == "FIELD" and decision.occupy ~= nil then
        local crew_busy = false
        for _, member in ipairs(party.members(snap)) do
            crew_busy = crew_busy or (member.errand ~= nil and member.self.ghost == false)
        end
        if crew_busy == false and require("brain.tasks.supply").ordered(snap, facts) == false then
            memory.serviced  = memory.servicing.at
            memory.servicing = nil
            brain:remember(bot, "serviced", memory.serviced)
            brain:remember(bot, "servicing", nil)
        end
    end
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
    local spot = snap.record.memory.spot
    if spot == nil or spot.map ~= snap.world.map or spot.x ~= snap.world.x or spot.y ~= snap.world.y then
        spot = { map = snap.world.map, x = snap.world.x, y = snap.world.y, at = snap.now }
        brain:remember(bot, "spot", spot)
        snap.record.memory.spot = spot
    end
    profile.measure(owner, "alert", alert.watch, brain, bot, snap, facts)
    local changed, commit, swept, solo = M.commit(snap, facts)
    if changed then
        brain:remember(bot, "commit", commit)
        brain:remember(bot, "swept", swept)
        brain:remember(bot, "solo", solo)
        snap.record.memory.commit = commit
        snap.record.memory.swept  = swept
        snap.record.memory.solo   = solo
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
    M.lap(brain, bot, snap, decision, facts)
    brain:post(bot, party.profile(snap, facts, decision))
    local moved, entry = M.entered(snap, facts)
    if moved then
        snap.record.memory.entry = entry
        brain:remember(bot, "entry", entry)
        if entry ~= nil then
            snap.record.memory.joined = entry.at
            brain:remember(bot, "joined", entry.at)
        end
    end
    local fled, released = M.flee(snap, decision)
    if fled ~= nil then
        brain:remember(bot, "fled", fled)
    end
    if released then
        for _, name in ipairs({ "commit", "solo", "aim" }) do
            snap.record.memory[name] = nil
            brain:remember(bot, name, nil)
        end
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
    if decision ~= nil and decision.looted ~= nil then
        snap.record.memory.looted = decision.looted
        brain:remember(bot, "looted", decision.looted)
    end
    if decision ~= nil and decision.shop ~= nil then
        snap.record.memory.shop = decision.shop
        brain:remember(bot, "shop", decision.shop)
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

local paused = {}

function M.decide(brain, bot)
    local owner = bot:name()
    local held  = paused[owner]
    if held ~= nil then
        local left = (held.ends - clock()) // 1000
        if left > 0 and bot:hp() >= held.hp and bot:map() == held.map then
            held.decision.revisit = left
            return held.decision
        end
        paused[owner] = nil
    end
    profile.start(owner)
    local decision, revisit = M.first(brain, bot, nil)
    profile.finish(brain, bot, owner)
    if decision ~= nil and decision.pause ~= nil then
        paused[owner] = { ends = clock() + decision.pause * 1000, hp = bot:hp(), map = bot:map(), decision = decision }
    end
    return decision or { state = "PREPARE", reason = "idle", kind = ACTION.HOLD, revisit = revisit }
end

return M
