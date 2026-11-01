local expect = require("integration.lib.expect")

local M = {}

local TIMEOUT = 120000
local START   = { map = "국내성", x = 15, y = 107 }

local function stage(ctx, index)
    local bot = ctx:bot(index)
    bot:unequip("WEAPON")
    bot:clear_inventory()
    bot:clear_all_spells()
    bot:change_class("전사")
    bot:chat("/레벨바꾸기 5")
    bot:chat("/힘바꾸기 10")
    local current = id2map(bot:map())
    bot:map_move(START.map, START.x, START.y)
    if current == nil or current:name() ~= START.map then
        if expect.arrive(ctx, index, START.map, TIMEOUT) == false then
            return nil
        end
    end
    return ctx:bot(index)
end

local function history(mind, bot)
    local lines = {}
    local tasks = {}
    for _, entry in ipairs(mind:states(bot) or {}) do
        table.insert(lines, string.format("%s/%s(%s)", entry.name, entry.task, entry.reason))
        tasks[entry.task] = (tasks[entry.task] or 0) + 1
    end
    return table.concat(lines, " -> "), tasks
end

local function finish(ctx, index, mind, name, passed, detail)
    local summary = string.format("%s history=%s", detail, (history(mind, ctx:bot(index))))
    expect.release(ctx, index, mind, TIMEOUT)
    if passed then
        log("debug", string.format("[brain:state:%s] SUCCESS %s", name, summary))
    else
        log("fatal", string.format("[brain:state:%s] FAILED %s", name, summary))
    end
    return passed
end

function M.idle(ctx, index)
    if stage(ctx, index) == nil then
        return false
    end
    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1, persona = { rush = 1 } })
    local idle = expect.within(ctx, TIMEOUT, function()
        local state = mind:state(ctx:bot(index))
        return state ~= nil and state.reason == "idle"
    end)
    return finish(ctx, index, mind, "idle", idle, "idle=" .. tostring(idle))
end

function M.equip(ctx, index)
    local bot = stage(ctx, index)
    if bot == nil then
        return false
    end
    bot:create_item("목도", 1)
    bot:equip(bot:item_slot("목도"))
    bot:create_item("목검", 1)

    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1, persona = { rush = 1 } })
    local worn = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        return current:item_count("목검") == 0 and current:item_count("목도") == 1
    end)
    local _, tasks = history(mind, ctx:bot(index))
    return finish(ctx, index, mind, "equip", worn and tasks.EQUIP ~= nil, "worn=" .. tostring(worn))
end

function M.unreachable(ctx, index)
    if stage(ctx, index) == nil then
        return false
    end
    ctx:bot(index):chat("/레벨바꾸기 1")
    local mind  = brain.new()
    local order = { task = "GO", dest = { map = "국내문파대결장", x = 15, y = 3 } }
    ctx:bot(index):brain(mind, { seed = 1, persona = { rush = 1 }, memory = { order = order } })
    local recorded = expect.within(ctx, TIMEOUT, function()
        local record = mind:failure(ctx:bot(index), "ORDER")
        return record ~= nil and record.reason == "NO_ROUTE"
    end)
    local quiet = expect.within(ctx, TIMEOUT, function()
        local state = mind:state(ctx:bot(index))
        return state ~= nil and state.reason == "idle"
    end)
    ctx:sleep(3000)
    local _, tasks = history(mind, ctx:bot(index))
    local passed   = recorded and quiet and tasks.ORDER == 1
    return finish(ctx, index, mind, "unreachable", passed,
        string.format("recorded=%s quiet=%s orders=%d", tostring(recorded), tostring(quiet), tasks.ORDER or 0))
end

function M.reattach(ctx, index)
    if stage(ctx, index) == nil then
        return false
    end
    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1, persona = { rush = 1 }, memory = { note = 7 } })
    expect.release(ctx, index, mind, TIMEOUT)
    ctx:bot(index):brain(mind, { seed = 1, persona = { rush = 1 } })
    expect.within(ctx, TIMEOUT, function()
        return mind:state(ctx:bot(index)) ~= nil
    end)
    local note = mind:recall(ctx:bot(index), "note")
    return finish(ctx, index, mind, "reattach", note == 7, "note=" .. tostring(note))
end

return M
