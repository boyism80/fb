local gear   = require("integration.lib.gear")
local expect = require("integration.lib.expect")
local ghost  = require("integration.scenarios.brain.stage.ghost")

local M = {}

local TIMEOUT    = 120000
local GROUND     = { map = "고구려왕초보사냥1", x = 9, y = 15 }
local WEAPON     = "목도"
local MONEY      = 1000
local MAX_MP     = 1000
local EQUIP_WAIT = 5000

local function stage(ctx, index, case)
    local bot = ctx:bot(index)
    gear.strip(bot, { "WEAPON" })
    bot:clear_inventory()
    bot:clear_all_spells()
    bot:change_class("전사")
    bot:chat("/레벨바꾸기 5")
    for _, name in ipairs(case.spells or {}) do
        bot:learn_spell(name)
    end
    bot:create_item(WEAPON, 1)
    bot:equip(bot:item_slot(WEAPON))
    local worn = expect.within(ctx, EQUIP_WAIT, function()
        return ctx:bot(index):item_count(WEAPON) == 0
    end)
    if worn == false then
        return nil
    end

    local current = id2map(bot:map())
    bot:map_move(GROUND.map, GROUND.x, GROUND.y)
    if current == nil or current:name() ~= GROUND.map then
        if expect.arrive(ctx, index, GROUND.map, TIMEOUT) == false then
            return nil
        end
    end
    bot = ctx:bot(index)
    bot:chat(string.format("/금전 %d", MONEY))
    bot:set_max_hp_mp(bot:base_hp(), MAX_MP)
    return bot
end

function M.run(ctx, index, case)
    local bot = stage(ctx, index, case)
    if bot == nil then
        log("fatal", string.format("[brain:recover:%s] FAILED stage", case.name))
        return false
    end

    local mind = brain.new()
    local died = ghost.die(ctx, index, case)
    ctx:bot(index):brain(mind, { seed = 1 })

    local alive   = false
    local again   = false
    local settled = died and expect.settle(ctx, index, mind, TIMEOUT, function(current)
        return current:state() ~= "GHOST" and mind:death(current) == nil and current:item_count(WEAPON) == 0
    end, function(current)
        local ghost = current:state() == "GHOST"
        again       = again or (alive and ghost)
        alive       = alive or ghost == false
    end)

    bot          = ctx:bot(index)
    local money  = bot:money()
    local bagged = bot:item_count(WEAPON)
    local jumped = mind:observe(bot).jumped
    local lines  = {}
    for _, entry in ipairs(mind:states(bot) or {}) do
        table.insert(lines, string.format("%s/%s(%s)", entry.name, entry.task, entry.reason))
    end
    local recovered = false
    for _, entry in ipairs(mind:states(bot) or {}) do
        recovered = recovered or entry.task == "RECOVER"
    end
    expect.release(ctx, index, mind, TIMEOUT)
    local cleared = ghost.clear(ctx, index)
    local kept    = case.guard ~= nil or ((case.money or MONEY) > 0) == (money > 0)
    local passed  = cleared and settled and recovered and again == false and kept and bagged == 0 and
        (case.jump == "*" or jumped == case.jump)
    local summary = string.format("died=%s settled=%s again=%s money=%d weapon_in_bag=%d jumped=%s history=%s",
        tostring(died), tostring(settled), tostring(again), money, bagged, tostring(jumped), table.concat(lines, " -> "))
    if passed then
        log("debug", string.format("[brain:recover:%s] SUCCESS %s", case.name, summary))
    else
        log("fatal", string.format("[brain:recover:%s] FAILED %s", case.name, summary))
    end
    return passed
end

return M
