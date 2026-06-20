local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local ITEM_NAME  = "도토리"
local ITEM_COUNT = 200
local MONEY_AMOUNT = 10000

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "LOOT SPELL TEST STARTED")

    caster:create_item(ITEM_NAME, ITEM_COUNT)
    caster:money(MONEY_AMOUNT)

    caster:move("BOTTOM")

    caster:request(
        resp.item_remove,
        protocol.item_drop(1, true),
        function(packet)
            return packet.index == 0
        end)
    caster:request(
        resp.update_internal,
        protocol.item_drop_money(MONEY_AMOUNT),
        function(packet)
            return packet.ch_money == 0
        end)

    caster:move("TOP")
    caster:direction("BOTTOM")

    caster:learn_spells({"노획"})
    caster:set_current_hp_mp(10000, 1000)

    local before_mp = caster:mp()
    local expected_mp = before_mp - 30

    caster:request(
        resp.item_update,
        protocol.spell_cast("NORMAL", 0, "", 0, {0, 0}),
        function(packet)
            return string.find(packet.name, ITEM_NAME, 1, true) == 1 and packet.count == ITEM_COUNT
        end)

    local after_mp = caster:mp()
    if after_mp ~= expected_mp then
        error(string.format(
            "Loot spell MP consumption verification failed: expected %d, got %d",
            expected_mp, after_mp))
    end

    log("debug", "LOOT SPELL TEST COMPLETED")
    return true
end

return M
