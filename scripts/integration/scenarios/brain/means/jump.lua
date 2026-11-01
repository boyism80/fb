local expect = require("integration.lib.expect")

local M = {}

local TIMEOUT  = 180000
local MAX_HP   = 1000
local MAX_MP   = 1000
local SPELL_MP = 30

local function stage(ctx, index, case)
    local bot = ctx:bot(index)
    bot:clear_inventory()
    bot:clear_all_spells()
    bot:chat("/자연회복 끄기")
    bot:set_max_hp_mp(MAX_HP, MAX_MP)
    for _, name in ipairs(case.spells or {}) do
        bot:learn_spell(name)
    end
    for _, name in ipairs(case.items or {}) do
        bot:create_item(name, 1)
    end

    local current = id2map(bot:map())
    if current == nil or current:name() ~= case.start.map then
        bot:map_move(case.start.map, case.start.x, case.start.y)
        local arrived
        arrived, bot = expect.arrive(ctx, index, case.start.map, TIMEOUT)
        if arrived == false then
            return nil
        end
    else
        bot:map_move(case.start.map, case.start.x, case.start.y)
    end
    bot = ctx:bot(index)
    bot:set_current_hp_mp(MAX_HP, case.mp or 0)
    return bot
end

function M.run(ctx, index, case)
    local bot = stage(ctx, index, case)
    if bot == nil then
        log("fatal", string.format("[brain:means:%s] FAILED stage", case.name))
        return false
    end

    local mp   = bot:mp()
    local mind = brain.new()
    bot:brain(mind, { seed = 1, persona = { rush = 1 }, memory = { order = { task = "GO", dest = case.dest } } })
    local done = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        return mind:recall(current, "order") == nil
    end)

    bot          = ctx:bot(index)
    local landed = id2map(bot:map())
    local here   = bot:position()
    local spent  = mp - bot:mp()
    local items  = {}
    for _, name in ipairs(case.items or {}) do
        items[name] = bot:item_count(name)
    end
    expect.release(ctx, index, mind, TIMEOUT)
    bot = ctx:bot(index)
    bot:chat("/자연회복 켜기")

    local arrived = landed ~= nil and landed:name() == (case.dest.map or "")
    if arrived and case.dest.x ~= nil then
        arrived = math.abs(here[1] - case.dest.x) + math.abs(here[2] - case.dest.y) <= (case.dest.range or 0)
    end
    local cast = spent >= SPELL_MP
    local used = true
    for name, count in pairs(items) do
        used = used and count == 0
    end
    local passed = done and arrived and cast == (case.cast or false) and
        (case.items == nil or used == (case.use or false))
    local summary = string.format("done=%s map=%s %d,%d mp_spent=%d cast=%s items_used=%s", tostring(done),
        landed and landed:name() or "?", here[1], here[2], spent, tostring(cast), tostring(used))
    if passed then
        log("debug", string.format("[brain:means:%s] SUCCESS %s", case.name, summary))
    else
        log("fatal", string.format("[brain:means:%s] FAILED %s", case.name, summary))
    end
    return passed
end

return M
