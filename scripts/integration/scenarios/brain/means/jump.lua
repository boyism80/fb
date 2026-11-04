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
    bot:change_class("전사")
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

    local mp      = bot:mp()
    local mind    = brain.new()
    local landed  = nil
    local here    = nil
    local spent   = 0
    local held    = {}
    local items   = {}
    local seen    = false
    local cleared = false
    bot:brain(mind, { seed = 1, memory = { order = { task = "GO", dest = case.dest } } })
    local done = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        return mind:recall(current, "order") == nil
    end, function(current)
        local pending = mind:recall(current, "order") ~= nil
        local model   = id2map(current:map())
        local inside  = model ~= nil and model:name() == (case.dest.map or "")
        seen          = seen or pending
        if landed == nil and seen and inside then
            landed = model
            here   = current:position()
        end
        if cleared == false and pending then
            for _, name in ipairs(case.items or {}) do
                held[name] = current:item_count(name)
            end
        end
        if cleared == false and seen and pending == false then
            cleared = true
            spent   = mp - current:mp()
            for _, name in ipairs(case.items or {}) do
                items[name] = case.use == false and held[name] or current:item_count(name)
            end
        end
    end)

    expect.release(ctx, index, mind, TIMEOUT)
    bot = ctx:bot(index)
    bot:chat("/자연회복 켜기")

    local arrived = cleared
    local cast = spent >= SPELL_MP
    local used = true
    for name, count in pairs(items) do
        used = used and count == 0
    end
    local passed = done and arrived and cast == (case.cast or false) and
        (case.items == nil or used == (case.use or false))
    local summary = string.format("done=%s map=%s %d,%d mp_spent=%d cast=%s items_used=%s", tostring(done),
        landed and landed:name() or "?", here ~= nil and here[1] or -1, here ~= nil and here[2] or -1, spent,
        tostring(cast), tostring(used))
    if passed then
        log("debug", string.format("[brain:means:%s] SUCCESS %s", case.name, summary))
    else
        log("fatal", string.format("[brain:means:%s] FAILED %s", case.name, summary))
    end
    return passed
end

return M
