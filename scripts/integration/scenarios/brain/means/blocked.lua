local expect      = require("integration.lib.expect")
local script_step = require("integration.lib.script_step")

local M = {}

local TIMEOUT = 180000
local TOWN    = "국내성"
local SLOT    = 8
local GROUND  = "고구려왕초보사냥1"
local START   = { x = 39, y = 176 }
local GATES   = { { x = 38, y = 172 }, { x = 39, y = 172 } }
local ASIDE   = { x = 45, y = 176 }
local BLOCKER = "좌성황"

local function place(ctx, index, x, y)
    local bot     = ctx:bot(index)
    local current = id2map(bot:map())
    if current == nil or current:name() ~= TOWN then
        bot:map_move(TOWN, x, y)
        if expect.arrive(ctx, index, TOWN, TIMEOUT) == false then
            return nil
        end
    end
    ctx:bot(index):map_move(TOWN, x, y, SLOT)
    return ctx:bot(index)
end

function M.run(ctx, index, other, case)
    local walker = place(ctx, index, START.x, START.y)
    local guard  = walker and place(ctx, other, GATES[1].x, GATES[1].y)
    if guard == nil then
        log("fatal", string.format("[brain:blocked:%s] FAILED stage", case.name))
        return false
    end
    if case.both then
        guard:create_npc(BLOCKER)
    end
    guard = place(ctx, other, GATES[2].x, GATES[2].y)

    walker = ctx:bot(index)
    walker:change_class("전사")
    walker:clear_all_spells()
    local mind = brain.new()
    walker:brain(mind,
        { seed = 1, persona = { rush = 1 }, memory = { order = { task = "GO", dest = { map = GROUND } } } })
    if case.both then
        expect.within(ctx, TIMEOUT, function()
            return mind:recall(ctx:bot(index), "blocked") ~= nil
        end)
        place(ctx, other, ASIDE.x, ASIDE.y)
    end

    local arrived = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        local model = id2map(current:map())
        return model ~= nil and model:name() == GROUND
    end)
    local blocked = mind:recall(ctx:bot(index), "blocked")
    expect.release(ctx, index, mind, TIMEOUT)
    if case.both then
        local cleaner = place(ctx, other, GATES[1].x, GATES[1].y + 1)
        script_step.run_script(cleaner, "test/integration", "remove_npc", 1, BLOCKER)
    end
    place(ctx, other, ASIDE.x, ASIDE.y)

    local passed  = arrived and blocked ~= nil
    local summary = string.format("arrived=%s blocked=%s", tostring(arrived), tostring(blocked))
    if passed then
        log("debug", string.format("[brain:blocked:%s] SUCCESS %s", case.name, summary))
    else
        log("fatal", string.format("[brain:blocked:%s] FAILED %s", case.name, summary))
    end
    return passed
end

return M
