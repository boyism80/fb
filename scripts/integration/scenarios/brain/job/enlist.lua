local expect   = require("integration.lib.expect")
local protocol = require("integration.protocol")
local resp     = require("integration.response")

local M = {}

local TIMEOUT       = 240000
local START         = { map = "국내성", x = 15, y = 107 }
local OFFERING      = "도토리"
local INFO_WAIT     = 3000
local MOVE_ATTEMPTS = 2

local function stage(ctx, index, case)
    local bot = ctx:bot(index)
    bot:clear_inventory()
    bot:clear_all_spells()
    bot:change_class("평민")
    bot:chat("/레벨바꾸기 5")
    if case.offering > 0 then
        bot:create_item(OFFERING, case.offering)
    end
    if case.filler ~= nil then
        bot:fill_inventory(case.filler)
    end

    local arrived = false
    for _ = 1, MOVE_ATTEMPTS do
        local current = id2map(ctx:bot(index):map())
        local moved   = pcall(function()
            ctx:bot(index):map_move(START.map, START.x, START.y)
        end)
        arrived       = moved and
            ((current ~= nil and current:name() == START.map) or expect.arrive(ctx, index, START.map, TIMEOUT))
        if arrived then
            break
        end
    end
    if arrived == false then
        return nil
    end
    bot = ctx:bot(index)
    bot:request(resp.internal_info, protocol.self_info(), function() return true end, INFO_WAIT)
    return bot
end

function M.growth(ctx, index)
    local bot = stage(ctx, index, { offering = 10 })
    if bot == nil then
        log("fatal", "[brain:growth] FAILED stage")
        return false
    end
    bot:chat("/금전 10000")
    local mind = brain.new()
    bot:brain(mind, { seed = 1, memory = { population = { WARRIOR = 25, ROGUE = 25, MAGE = 17, POET = 0 } } })
    local joined  = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        return mind:observe(current).class ~= "NONE"
    end)
    local learned = joined and expect.settle(ctx, index, mind, TIMEOUT, function(current)
        return current:spell_slot("비영사천문") ~= 0xFF and current:spell_slot("성황령") ~= 0xFF
    end)
    local current = ctx:bot(index)
    local passed  = joined and learned
    log(passed and "debug" or "fatal", string.format(
        "[brain:growth] %s class=%s cardinal=%s revive=%s", passed and "SUCCESS" or "FAILED",
        tostring(mind:observe(current).class), tostring(current:spell_slot("비영사천문") ~= 0xFF),
        tostring(current:spell_slot("성황령") ~= 0xFF)))
    expect.release(ctx, index, mind, TIMEOUT)
    return passed
end

function M.run(ctx, index, case)
    local bot = stage(ctx, index, case)
    if bot == nil then
        log("fatal", string.format("[brain:job:%s] FAILED stage", case.name))
        return false
    end

    local mind = brain.new()
    bot:brain(mind, { seed = 1, memory = { population = case.population } })
    local held = false
    local joined = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        local record = mind:failure(current, "JOB")
        held         = held or (record ~= nil and record.reason == "FULL")
        if case.stuck then
            return held
        end
        return mind:observe(current).class ~= "NONE" and mind:recall(current, "job") == nil
    end)
    local attacked = false
    for _, activity in ipairs(mind:activities(ctx:bot(index)) or {}) do
        attacked = attacked or activity == "attack"
    end

    bot           = ctx:bot(index)
    local class   = mind:observe(bot).class
    local failure = mind:state(bot)
    local reasons = {}
    for _, task in ipairs({ "JOB", "LOOT", "FIELD", "REST" }) do
        local record = mind:failure(bot, task)
        if record ~= nil then
            table.insert(reasons, string.format("%s:%s*%s", task, tostring(record.reason), tostring(record.failures)))
        end
    end
    local reserved = mind:reservations(bot)
    local pending  = mind:recall(bot, "job")
    local lines    = {}
    local enlisted = false
    for _, entry in ipairs(mind:states(bot) or {}) do
        table.insert(lines, string.format("%s/%s(%s)", entry.name, entry.task, entry.reason))
        enlisted = enlisted or entry.task == "JOB"
    end
    expect.release(ctx, index, mind, TIMEOUT)

    local passed = joined and enlisted and class == case.expected and pending == nil
    if case.stuck then
        passed = joined and held and class == "NONE" and attacked == false
    end
    local summary = string.format("failures=%s %s joined=%s class=%s expected=%s pending=%s reserved=%d history=%s",
        tostring(failure and failure.failures), table.concat(reasons, ","), tostring(joined),
        class, tostring(case.expected), tostring(pending), next(reserved) == nil and 0 or 1, table.concat(lines, " -> "))
    if passed then
        log("debug", string.format("[brain:job:%s] SUCCESS %s", case.name, summary))
    else
        log("fatal", string.format("[brain:job:%s] FAILED %s", case.name, summary))
    end
    return passed
end

return M
