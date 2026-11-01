local expect = require("integration.lib.expect")

local M = {}

local TIMEOUT = 240000
local GROUND  = { map = "고구려왕초보사냥1", x = 9, y = 15, slot = 7 }
local WEAPON  = "목도"
local MAX_HP  = 1000
local MAX_MP  = 1000

local function stage(ctx, index, case)
    local bot = ctx:bot(index)
    bot:clear_inventory()
    bot:clear_all_spells()
    bot:change_class("전사")
    bot:create_item(WEAPON, 1)
    bot:equip(bot:item_slot(WEAPON))
    for _, name in ipairs(case.items or {}) do
        bot:create_item(name, 1)
    end

    local current = id2map(bot:map())
    if current == nil or current:name() ~= GROUND.map then
        bot:map_move(GROUND.map, GROUND.x, GROUND.y)
        if expect.arrive(ctx, index, GROUND.map, TIMEOUT) == false then
            return nil
        end
    end
    bot = ctx:bot(index)
    local start = case.start or GROUND
    bot:map_move(GROUND.map, start.x, start.y, GROUND.slot)
    bot = ctx:bot(index)
    bot:chat("/몬스터제거")
    bot:chat("/리스폰")
    bot:set_max_hp_mp(MAX_HP, MAX_MP)
    bot:set_current_hp_mp(case.hp, MAX_MP)
    if case.monster ~= nil then
        local here = bot:position()
        local offset = case.offset or { 2, 0 }
        bot:chat(string.format("/몬스터생성 %s %d %d", case.monster, here[1] + offset[1], here[2] + offset[2]))
    end
    return bot
end

function M.run(ctx, index, case)
    local bot = stage(ctx, index, case)
    if bot == nil then
        log("fatal", string.format("[brain:danger:%s] FAILED stage", case.name))
        return false
    end

    local mind  = brain.new()
    local ghost = false
    bot:brain(mind, { seed = 1, persona = { rush = 1 }, memory = { order = case.order } })
    local done = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        ghost = ghost or current:state() == "GHOST"
        return (case.keep or mind:recall(current, "order") == nil) and case.finished(current)
    end)

    bot          = ctx:bot(index)
    local landed = id2map(bot:map())
    local jumped = mind:observe(bot).jumped
    local lines  = {}
    local tasks  = {}
    for _, entry in ipairs(mind:states(bot) or {}) do
        table.insert(lines, string.format("%s/%s(%s)", entry.name, entry.task, entry.reason))
        tasks[entry.task] = true
    end
    expect.release(ctx, index, mind, TIMEOUT)
    local cleaner = ctx:bot(index)
    cleaner:map_move(GROUND.map, GROUND.x, GROUND.y)
    expect.arrive(ctx, index, GROUND.map, TIMEOUT)
    ctx:bot(index):map_move(GROUND.map, GROUND.x, GROUND.y, GROUND.slot)
    ctx:bot(index):chat("/몬스터제거")

    local passed = done and ghost == false and jumped == case.jump
    for _, task in ipairs(case.tasks or {}) do
        passed = passed and tasks[task] == true
    end
    local summary = string.format("done=%s ghost=%s map=%s jumped=%s history=%s", tostring(done), tostring(ghost),
        landed and landed:name() or "?", tostring(jumped), table.concat(lines, " -> "))
    if passed then
        log("debug", string.format("[brain:danger:%s] SUCCESS %s", case.name, summary))
    else
        log("fatal", string.format("[brain:danger:%s] FAILED %s", case.name, summary))
    end
    return passed
end

return M
