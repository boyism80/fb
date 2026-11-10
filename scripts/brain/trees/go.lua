local castable = require("brain.policies.combat")
local combat   = require("brain.data.combat")
local teleport = require("brain.data.teleport")
local tree     = require("brain.tree")

local M = {}

local function jump_of(entry, snap, urgent, leaving)
    local cost = leaving and 0 or (urgent or snap.self.ghost) and 1 or
        math.ceil(combat.transfer / ((snap.pace or {}).step or combat.walk))
    return {
        spell  = entry.spell ~= nil and entry.name or nil,
        item   = entry.spell == nil and entry.name or nil,
        via    = entry.via,
        to     = entry.returns and teleport.returns[snap.self.nation] or entry.to,
        inputs = entry.inputs,
        cost   = cost,
    }
end

local function learned(name)
    return function(ctx)
        for _, spell in ipairs(ctx.spells or {}) do
            if spell.name == name then
                return true
            end
        end
        return false
    end
end

local function held(name)
    return function(ctx)
        for _, item in ipairs(ctx.bag.items) do
            if item.name == name and item.count > 0 then
                return true
            end
        end
        return false
    end
end

local function ghost_match(entry)
    return function(ctx)
        return (entry.ghost == true) == ctx.self.ghost
    end
end

local function enough_mp(entry)
    return function(ctx)
        return (ctx.self.mp or 0) >= (entry.mp or 0)
    end
end

local function alive(ctx)
    return ctx.self.ghost == false
end

local function unspent(name)
    return function(ctx)
        return ctx.go.spent[name] == nil
    end
end

local function preferred(ctx, jump, option)
    if ctx.go.walk == nil then
        return true
    end
    if ctx.go.leaving ~= true then
        return option.cost < ctx.go.walk
    end
    return jump.via == "CARDINAL" or ctx.go.trapped or
        ctx.go.walk > math.ceil(combat.transfer / ((ctx.pace or {}).step or combat.walk))
end

local function faster_option(name)
    return function(ctx)
        local best = nil
        local via  = nil
        for _, option in ipairs(ctx.go.options) do
            local jump = ctx.go.jumps[option.index]
            if (jump.spell == name or jump.item == name) and preferred(ctx, jump, option) and
                (best == nil or option.cost < best.cost) then
                best = option
                via  = jump.via
            end
        end
        if best == nil then
            return nil
        end
        return {
            first = ctx.go.leaving == true and via == "CARDINAL",
            name = name,
            index = best.index,
            side = best.side,
            dest = best.dest,
            x = best.x,
            y = best.y,
            cost = best
                .cost
        }
    end
end

local entries = {}
local nodes   = {}
for _, spell in ipairs(teleport.spells) do
    local entry = setmetatable({ spell = true }, { __index = spell })
    entry.usable = function(ctx)
        return castable.can_cast(ctx) and learned(spell.name)(ctx) and ghost_match(entry)(ctx) and enough_mp(entry)(ctx)
    end
    table.insert(entries, entry)
    table.insert(nodes, tree.when(unspent(spell.name), tree.node(spell.name, faster_option(spell.name))))
end
for _, item in ipairs(teleport.items) do
    local entry = setmetatable({}, { __index = item })
    entry.usable = function(ctx)
        return alive(ctx) and held(item.name)(ctx)
    end
    table.insert(entries, entry)
    table.insert(nodes, tree.when(unspent(item.name), tree.node(item.name, faster_option(item.name))))
end

local choice = tree.pick(nodes, function(_, _, found)
    return found.first and math.huge or -found.cost
end, "go_leap")

local function leaving(snap, facts, destination)
    if destination.leaving then
        return true
    end
    local plan = require("brain.policies.party").plan(snap, facts)
    if plan == nil then
        return false
    end
    local here = false
    local there = false
    local maps = destination.maps or { destination.map }
    for _, floor in ipairs(plan.floors) do
        here = here or floor.map == snap.world.map
        for _, map in ipairs(maps) do
            there = there or floor.map == map
        end
    end
    return here and there == false
end

function M.jumps(snap, urgent, leaving)
    local jumps = {}
    for _, entry in ipairs(entries) do
        if entry.usable(snap) then
            table.insert(jumps, jump_of(entry, snap, urgent, leaving))
        end
    end
    return jumps
end

-- A walk of a few steps is never a trip: a jump lands at its own spot, not the steps closer. Taken for the
-- one step a poet closed in between casts, it cast 비영사천문 twice (online group, 16:22).
function M.choose(snap, facts, destination, detour)
    detour.leaving = leaving(snap, facts, destination)
    detour.jumps   = destination.steps == nil and M.jumps(snap, destination.urgent, detour.leaving) or {}
    detour.spent   = detour.spent or {}
    detour.leap    = nil
    detour.side    = nil
    if #detour.jumps == 0 then
        return nil
    end
    local found = facts.leaps(destination, detour,
        { map = snap.world.map, x = snap.world.x, y = snap.world.y, level = snap.self.level })
    if found == nil then
        return nil
    end
    local ctx    = setmetatable({
        go = {
            spent = detour.spent,
            jumps = detour.jumps,
            options = found.options,
            walk = found.walk,
            leaving = detour.leaving,
            trapped = require("brain.policies.party").stalled(snap, (snap.pace or {}).swing or combat.swing)
        },
    }, { __index = snap })
    local chosen = tree.run(choice, ctx, facts)
    if chosen ~= nil then
        detour.leap = chosen.index
        detour.side = chosen.side
    end
    return chosen
end

return M
