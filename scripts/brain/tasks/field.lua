local data     = require("brain.data.combat")
local battle   = require("brain.policies.battle")
local combat   = require("brain.policies.combat")
local escort   = require("brain.policies.escort")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local value    = require("brain.policies.value")

local M = {}

local journaled = {}

local function position(snap, floors)
    for index, floor in ipairs(floors) do
        if floor.map == snap.world.map then
            return index
        end
    end
    return nil
end

function M.castable(snap, query, mob)
    local least = recovery.least(snap)
    if least == nil or (snap.self.mp or 0) < least or snap.self.class == "WARRIOR" then
        return true
    end
    for _, choice in ipairs(combat.options(snap, mob).offense) do
        if choice.damage > 0 then
            return true
        end
    end
    return false
end

local function allowed(snap, mob)
    local distance = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
    if distance > 1 and combat.hits(mob) and snap.self.hp <= combat.spike(snap, { { mob = mob, count = 1 } }) then
        return false
    end
    local range = math.min(data.sight.x, data.sight.y)
    for _, member in ipairs(party.members(snap)) do
        local world = member.world
        if member.self.ghost == false and world.map == snap.world.map and world.x ~= nil and
            math.abs(mob.x - world.x) + math.abs(mob.y - world.y) > range then
            return false
        end
    end
    return true
end

function M.target(snap, query, names)
    local wanted = {}
    for _, name in ipairs(names) do
        wanted[name] = true
    end
    local lead   = snap.leader ~= snap.self.name and party.leader(snap) or nil
    local shared = lead ~= nil and lead.battle or nil
    if type(shared) == "table" and shared.map == snap.world.map then
        local found = battle.target(snap, query, function(mob)
            return mob.oid == shared.oid and wanted[mob.name] ~= nil and M.castable(snap, query, mob) and
                allowed(snap, mob)
        end)
        if found ~= nil then
            return found
        end
    end
    return battle.target(snap, query, function(mob)
        return wanted[mob.name] ~= nil and M.castable(snap, query, mob) and allowed(snap, mob)
    end)
end

local function area(snap, query, names)
    local found = nil
    local best  = nil
    local range = math.min(data.sight.x, data.sight.y)
    for _, spawn in ipairs(query.spawns(snap.world.map, names)) do
        local cost = math.abs(spawn.x - snap.world.x) + math.abs(spawn.y - snap.world.y)
        if cost > range and (best == nil or cost < best) then
            found = spawn
            best  = cost
        end
    end
    return found
end

local function recoiling(snap, query, dest)
    local fleeing = snap.record.memory.fleeing
    if fleeing == nil or snap.now >= fleeing.ends or fleeing.map == snap.world.map or dest.maps == nil then
        return false
    end
    local route = query.route(dest.maps)
    local hop   = route ~= nil and route.path ~= nil and route.path[1] or nil
    return hop ~= nil and hop.dest == fleeing.map
end

function M.revisit(snap)
    local fleeing = snap.record.memory.fleeing
    return fleeing ~= nil and fleeing.ends > snap.now and fleeing.ends - snap.now or nil
end

function M.wanted(snap, query)
    if snap.self.ghost then
        return false
    end
    local objective = value.objective(snap, query)
    if objective.task ~= "FIELD" or objective.dest == nil or recoiling(snap, query, objective.dest) then
        return false
    end
    local plan = party.plan(snap, query)
    local crew = escort.present(snap, query)
    if data.journal and plan ~= nil and journaled[snap.self.name or ""] ~= plan.name then
        journaled[snap.self.name or ""] = plan.name
        local others = {}
        for _, member in ipairs(party.members(snap)) do
            table.insert(others, string.format("%s:%s/%s:%s:%s", member.self.level, member.self.hp, member.self.mp,
                tostring(member.world.map), table.concat(member.buffs or {}, "+")))
        end
        log("debug",
            string.format("[brain:plan] bot=%s lv=%s map=%s plan=%s rate=%.0f hp=%s mp=%s commit=%s buffs=%s others=%s",
                snap.self.name or "", tostring(snap.self.level), tostring(snap.world.map), plan.name, plan.rate,
                tostring(snap.self.hp), tostring(snap.self.mp), tostring(snap.record.memory.commit),
                table.concat(snap.buffs or {}, "+"), table.concat(others, ";")))
        local worn = {}
        for _, slot in ipairs(snap.gear or {}) do
            table.insert(worn, slot.names[1])
        end
        local held = {}
        for _, item in ipairs(snap.bag.items or {}) do
            table.insert(held, item.name .. "x" .. item.count)
        end
        local learned = 0
        for _ in ipairs(snap.spells or {}) do
            learned = learned + 1
        end
        log("debug",
            string.format(
                "[brain:plan] detail max=%s/%s str=%s dex=%s int=%s hit=%s dam=%s def=%s money=%s spells=%d step=%s swing=%s demand=%s gear=%s bag=%s",
                tostring(snap.self.max_hp), tostring(snap.self.max_mp), tostring(snap.self.str), tostring(snap.self.dex),
                tostring(snap.self.int),
                tostring(snap.self.hit), tostring(snap.self.dam), tostring(snap.self.phydef), tostring(snap.self.money),
                learned,
                tostring(snap.pace.step), tostring(snap.pace.swing), tostring(query.demand(plan.floors[1].map)),
                table.concat(worn, ","), table.concat(held, ",")))
    end
    local hp = snap.self.hp + (crew ~= nil and crew.heal or 0) * combat.cast(snap) / 1000
    return snap.self.hp >= snap.self.max_hp or hp > (plan ~= nil and plan.peak or 0)
end

local function stage(snap, query, maps)
    local route = query.route(maps)
    local exit  = route ~= nil and route.path ~= nil and route.path[1] or nil
    if exit == nil or exit.x == nil then
        return { kind = ACTION.MOVE, dest = { maps = maps } }
    end
    if math.abs(exit.x - snap.world.x) + math.abs(exit.y - snap.world.y) > data.adjacent then
        return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = exit.x, y = exit.y, range = data.adjacent } }
    end
    for _, member in ipairs(party.members(snap)) do
        local world   = member.world
        local through = false
        for _, map in ipairs(maps) do
            through = through or map == world.map
        end
        if member.self.ghost == false and through == false and (world.map ~= snap.world.map or world.x == nil or
                (member.task ~= "FIELD" and member.task ~= "NOTHING") or
                math.abs(exit.x - world.x) + math.abs(exit.y - world.y) > 2 * data.adjacent) then
            return { kind = ACTION.HOLD }
        end
    end
    return { kind = ACTION.MOVE, dest = { maps = maps } }
end

local function gather(snap, query, best, index)
    if snap.leader == snap.self.name then
        local gate = best.gate or best.floors[1].map
        if index == nil and snap.world.map ~= gate then
            return { kind = ACTION.MOVE, dest = { maps = { gate } } }
        end
        if index == nil then
            return stage(snap, query, { best.floors[1].map })
        end
        local range   = math.min(data.sight.x, data.sight.y)
        local nearest = nil
        local closest = nil
        for _, member in ipairs(party.members(snap)) do
            local world = member.world
            if member.self.ghost == false and world.map == snap.world.map and world.x ~= nil then
                local distance = math.abs(world.x - snap.world.x) + math.abs(world.y - snap.world.y)
                if distance > range and (closest == nil or distance < closest) then
                    nearest = member
                    closest = distance
                end
            end
        end
        if nearest ~= nil then
            return {
                kind = ACTION.MOVE,
                dest = {
                    map = snap.world.map,
                    x = nearest.world.x,
                    y = nearest.world.y,
                    range = range
                }
            }
        end
        return { kind = ACTION.HOLD }
    end
    local lead = party.leader(snap)
    local gate = best.gate or best.floors[1].map
    if lead == nil or lead.world.x == nil then
        return { kind = ACTION.HOLD }
    end
    if lead.world.map ~= snap.world.map and lead.heading == nil then
        local onplan = lead.world.map == gate
        for _, floor in ipairs(best.floors) do
            onplan = onplan or floor.map == lead.world.map
        end
        local meet = onplan and lead.world.map or gate
        return { kind = ACTION.MOVE, dest = { maps = { meet ~= snap.world.map and meet or lead.world.map } } }
    end
    local arrived = lead.heading == nil
    for _, map in ipairs(lead.heading or {}) do
        arrived = arrived or map == snap.world.map
    end
    if arrived == false then
        return { kind = ACTION.MOVE, dest = { maps = lead.heading } }
    end
    local range = data.adjacent
    if lead.world.map ~= snap.world.map or math.abs(lead.world.x - snap.world.x) + math.abs(lead.world.y - snap.world.y) <= range then
        return { kind = ACTION.HOLD }
    end
    return { kind = ACTION.MOVE, dest = { map = lead.world.map, x = lead.world.x, y = lead.world.y, range = range } }
end

local function settled(snap)
    if #(snap.party or {}) > 0 and snap.leader ~= snap.self.name then
        return false
    end
    for _, member in ipairs(party.members(snap)) do
        if member.world.map == snap.world.map and member.self.ghost == false and member.task ~= nil and
            member.task ~= "FIELD" and member.task ~= "NOTHING" then
            return false
        end
    end
    return true
end

local function claim(best, action)
    local floor = best.floors[1]
    if floor.kills ~= nil then
        action.occupy = { map = floor.map, rate = floor.kills, name = best.name }
    end
    return action
end

function M.propose(snap, query)
    local best  = party.plan(snap, query)
    local index = position(snap, best.floors)
    if #(snap.party or {}) > 0 and (escort.complete(snap, query) == false or (index == nil and snap.leader ~= snap.self.name)) then
        return claim(best, gather(snap, query, best, index))
    elseif index == nil and #(snap.party or {}) > 0 then
        local gate = best.gate or best.floors[1].map
        if snap.world.map ~= gate and gate ~= best.floors[1].map then
            return claim(best, { kind = ACTION.MOVE, dest = { maps = { gate } } })
        end
        return claim(best, stage(snap, query, { best.floors[1].map }))
    elseif index == nil then
        return claim(best, { kind = ACTION.MOVE, dest = { maps = { best.floors[1].map } }, aim = best.name })
    end

    local floor  = best.floors[index]
    local occupy = {
        map = floor.map,
        rate = floor.kills,
        exp = floor.rate,
        deaths = floor.deaths,
        margin = floor.margin,
        wear = floor.wear,
        extra = floor.crowd,
        name = best.name,
        crew = #(snap.party or {})
    }
    local found  = M.target(snap, query, floor.targets)
    if found ~= nil then
        return battle.propose(found, { occupy = occupy })
    elseif #best.floors > 1 and settled(snap) == false then
        local lead = snap.leader ~= snap.self.name and party.leader(snap) or nil
        if lead ~= nil and lead.world.map == snap.world.map and lead.world.x ~= nil and
            math.abs(lead.world.x - snap.world.x) + math.abs(lead.world.y - snap.world.y) > data.adjacent then
            return {
                kind = ACTION.MOVE,
                dest = {
                    map = snap.world.map,
                    x = lead.world.x,
                    y = lead.world.y,
                    range = data.adjacent
                },
                occupy = occupy
            }
        end
        return { kind = ACTION.HOLD, occupy = occupy }
    elseif #best.floors > 1 then
        local next = best.floors[index % #best.floors + 1]
        local leg  = snap.record.memory.leg
        if leg ~= nil and leg.name == best.name and leg.map ~= snap.world.map then
            for _, entry in ipairs(best.floors) do
                if entry.map == leg.map then
                    next = entry
                end
            end
        end
        if #(snap.party or {}) > 0 then
            local action  = stage(snap, query, { next.map })
            action.occupy = occupy
            action.leg    = { name = best.name, map = next.map }
            return action
        end
        return {
            kind = ACTION.MOVE,
            dest = { maps = { next.map } },
            occupy = occupy,
            leg = { name = best.name, map = next.map }
        }
    end
    local spot = area(snap, query, floor.targets)
    if spot ~= nil then
        return {
            kind = ACTION.MOVE,
            dest = {
                map = snap.world.map,
                x = spot.x,
                y = spot.y,
                range = math.min(data.sight.x, data.sight.y)
            },
            occupy = occupy
        }
    end
    return { kind = ACTION.HOLD }
end

return M
