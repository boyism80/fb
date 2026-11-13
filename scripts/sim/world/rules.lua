local data    = require("brain.data.combat")
local magic   = require("brain.data.spells")
local ground  = require("brain.policies.ground")
local moments = require("sim.moments")

local M = {}

M.BEFORE  = 8
M.LASTING = 5000

local MOMENTARY = { strike_spell = true, fuel_spell = true, healer_walks = true }

local function closed(actor, state, name, began)
    began.to = actor.clock
    actor:emit({ kind = "broke", rule = name, from = began.from, to = began.to, map = began.map, detail = began.detail })
    local worst = state.worst[name]
    if worst == nil or began.to - began.from > worst.to - worst.from then
        state.worst[name] = began
    end
end

function M.note(actor, name, broken, detail)
    local state = actor.rules or { open = {}, worst = {} }
    actor.rules = state
    local began = state.open[name]
    if broken and began == nil then
        state.open[name] = {
            from   = actor.clock,
            map    = actor.active ~= nil and actor.active.map.id or nil,
            detail = detail,
            before = moments.lately(actor, M.BEFORE),
        }
    elseif broken == false and began ~= nil then
        state.open[name] = nil
        closed(actor, state, name, began)
    end
end

function M.open(actor)
    local found = {}
    for name, began in pairs(actor.rules ~= nil and actor.rules.open or {}) do
        table.insert(found, { rule = name, from = began.from })
    end
    table.sort(found, function(left, right)
        return left.rule < right.rule
    end)
    return found
end

local function sorted(entries)
    local names = {}
    for name in pairs(entries) do
        table.insert(names, name)
    end
    table.sort(names)
    return names
end

function M.flush(actor)
    local state = actor.rules
    if state == nil then
        return
    end
    for _, name in ipairs(sorted(state.open)) do
        closed(actor, state, name, state.open[name])
    end
    state.open  = {}
    local alarm = actor.options.alarm
    local bot   = string.gsub(actor.base.self.name or "", "[^%w_]", "_")
    for _, name in ipairs(sorted(state.worst)) do
        local worst  = state.worst[name]
        local lasted = worst.to - worst.from
        if alarm ~= nil and (MOMENTARY[name] or lasted >= M.LASTING) then
            local path = string.format("%s.broke.%s.%s.%s.moments", alarm, name, tostring(actor.options.tag), bot)
            if moments.write(path, worst.before) then
                actor:emit({ kind = "worst", rule = name, from = worst.from, to = worst.to, map = worst.map, file = path })
            end
        end
    end
    state.worst = {}
end

local function knows(spells, test)
    for _, learned in ipairs(spells or {}) do
        local spell = magic.spells[learned.name]
        if spell ~= nil and test(spell) then
            return true
        end
    end
    return false
end

local function strikes(spell)
    return spell.role == "STRIKE"
end

local mended = setmetatable({}, { __mode = "k" })

local function mend(spells)
    if spells == nil then
        return 0
    end
    if mended[spells] == nil then
        local best = 0
        for _, learned in ipairs(spells) do
            local spell = magic.spells[learned.name]
            if spell ~= nil and spell.role == "HEAL" and spell.self ~= true and type(spell.hp) == "number" then
                best = math.max(best, spell.hp)
            end
        end
        mended[spells] = best
    end
    return mended[spells]
end

local function harms(spell)
    return (spell.role == "ATTACK" or spell.role == "AREA") and (type(spell.mp) ~= "number" or spell.mp > 0)
end

local hurts = setmetatable({}, { __mode = "k" })

local function hurt(spells)
    if spells == nil then
        return 0
    end
    if hurts[spells] == nil then
        local best = 0
        for _, learned in ipairs(spells) do
            local spell = magic.spells[learned.name]
            if spell ~= nil and harms(spell) and type(spell.damage) == "number" then
                best = math.max(best, spell.damage)
            end
        end
        hurts[spells] = best
    end
    return hurts[spells]
end

local function mates(actor)
    local found = {}
    local group = actor.session:group(actor)
    for mate in pairs(group ~= nil and group.members or {}) do
        if mate ~= actor and mate.active ~= nil and mate.me.dead ~= true and mate.finished ~= true and mate.transit ~= true then
            table.insert(found, mate)
        end
    end
    return found
end

local function seen(actor, mate)
    return mate.active == actor.active and math.abs(mate.me.x - actor.me.x) <= data.sight.x and
        math.abs(mate.me.y - actor.me.y) <= data.sight.y
end

function M.check(actor, snap, decision)
    local state  = actor.rules or { open = {}, worst = {} }
    actor.rules  = state
    local me     = actor.me
    local alive  = me.dead ~= true and actor.active ~= nil
    local crew   = alive and mates(actor) or {}
    local kind   = decision ~= nil and decision.kind or nil
    local key    = decision ~= nil and decision.key or nil
    local cast   = kind == ACTION.ATTACK and key ~= nil and magic.spells[key] or nil
    local target = alive and kind == ACTION.ATTACK and decision.oid ~= nil and actor.active.mobs[decision.oid] or nil
    if state.acted == nil or (decision ~= nil and kind ~= ACTION.HOLD and kind ~= ACTION.TASK) then
        state.acted = actor.clock
    end

    local struck = 0
    local walked = false
    local dest   = kind == ACTION.MOVE and decision.dest or nil
    for _, mob in pairs(alive and actor.active.mobs or {}) do
        if mob.dead ~= true then
            if mob.locked == me and math.abs(mob.x - me.x) + math.abs(mob.y - me.y) == 1 then
                struck = struck + 1
            end
            walked = walked or (dest ~= nil and dest.map == actor.active.map.id and dest.x == mob.x and dest.y == mob.y)
        end
    end
    if target ~= nil and key == "SWING" and math.abs(target.x - me.x) + math.abs(target.y - me.y) > 1 then
        walked = true
    end

    local mine   = alive and mend(actor.spells) or 0
    local healer = #crew > 0 and mine > hurt(actor.spells)
    for _, mate in ipairs(crew) do
        healer = healer and mine > mend(mate.spells)
    end
    local striker = alive and knows(snap.spells, strikes)
    local hunting = alive and ground.floor_at(actor.active.map.id) ~= nil
    local apart   = false
    local hitter  = false
    local fed     = false
    local rooted  = false
    local drawn   = false
    for _, mate in ipairs(crew) do
        local near    = seen(actor, mate)
        local profile = actor.session.profiles[mate] or {}
        apart         = apart or (hunting and ground.floor_at(mate.active.map.id) ~= nil and near == false)
        hitter        = hitter or (near and mend(mate.spells) < mine)
        fed           = fed or (near and knows(mate.spells, strikes))
        rooted        = rooted or
            (hunting and mate.active == actor.active and near == false and (profile.pinned == true or profile.walled == true))
        drawn         = drawn or (target ~= nil and target.locked == mate.me)
    end

    local swing = snap.pace.swing or data.swing
    M.note(actor, "apart", apart)
    M.note(actor, "rooted_left", rooted)
    M.note(actor, "healer_struck", healer and struck > 0 and hitter, struck)
    M.note(actor, "struck_idle", struck > 0 and actor.clock - state.acted > 2 * swing, struck)
    M.note(actor, "strike_spell", striker and cast ~= nil and harms(cast) and drawn == false, key)
    M.note(actor, "fuel_spell", healer and cast ~= nil and harms(cast) and fed, key)
    M.note(actor, "healer_walks", healer and hitter and walked)
end

return M
