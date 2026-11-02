local expect   = require("integration.lib.expect")
local instance = require("integration.lib.instance")
local option   = require("integration.lib.option")
local protocol = require("integration.protocol")

local M = {}

local ARENA        = { map = "국내문파대결장", x = 15, y = 3 }
local SPELL        = "헬파이어"
local MANA         = 100
local TIMEOUT      = 60000
local VANISH_RANGE = 1
local GUARD_OFFSET = { 0, 3 }

local function warp(ctx, index, map, x, y, slot)
    local bot     = ctx:bot(index)
    local current = id2map(bot:map())
    bot:map_move(map, x, y, slot)
    if current == nil or current:name() ~= map then
        if expect.arrive(ctx, index, map, TIMEOUT) == false then
            return nil
        end
    end
    return ctx:bot(index)
end

function M.die(ctx, index, case)
    local bot    = ctx:bot(index)
    local origin = id2map(bot:map())
    local here   = bot:position()
    if origin == nil then
        return false
    end

    bot = warp(ctx, index, ARENA.map, ARENA.x, ARENA.y, case.slot)
    if bot == nil or (case.slot ~= nil and instance.inside(bot) == false) then
        return false
    end
    bot:chat("/몬스터제거")
    option.disable_pk_protect(bot)
    local slot = bot:learn_spell(SPELL)
    bot:set_max_hp_mp(bot:base_hp(), MANA)
    bot:set_current_hp_mp(1, MANA)
    bot:send(protocol.spell_cast("TARGET", slot, "", bot:oid(), bot:position()))
    local died = expect.within(ctx, TIMEOUT, function()
        return ctx:bot(index):state() == "GHOST"
    end)
    ctx:bot(index):chat(string.format("/마법지우기 %d", slot))
    if case.vanish then
        ctx:bot(index):chat(string.format("/아이템범위제거 %d", VANISH_RANGE))
    end
    if case.guard ~= nil then
        local corpse = ctx:bot(index):position()
        ctx:bot(index):chat(string.format("/몬스터생성 %s %d %d", case.guard, corpse[1] + GUARD_OFFSET[1],
            corpse[2] + GUARD_OFFSET[2]))
    end

    return died and warp(ctx, index, origin:name(), here[1], here[2]) ~= nil
end

function M.clear(ctx, index)
    local bot    = ctx:bot(index)
    local origin = id2map(bot:map())
    local here   = bot:position()
    bot          = warp(ctx, index, ARENA.map, ARENA.x, ARENA.y)
    if bot == nil then
        return false
    end
    bot:chat("/몬스터제거")
    bot:chat(string.format("/아이템범위제거 %d", VANISH_RANGE + GUARD_OFFSET[2]))
    return origin == nil or warp(ctx, index, origin:name(), here[1], here[2]) ~= nil
end

return M
