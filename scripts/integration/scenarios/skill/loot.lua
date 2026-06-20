local resp         = require("integration.response")
local protocol     = require("integration.protocol")
local spell_runner = require("integration.lib.spell_runner")

local M = {}

local ITEM_NAME    = "도토리"
local ITEM_COUNT   = 200
local MONEY_AMOUNT = 10000

local CASES = {
    {
        name = "노획",
        response = resp.item_update,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
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
            caster:set_current_hp_mp(10000, 1000)
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.name ~= nil
                and string.find(packet.name, ITEM_NAME, 1, true) == 1
                and packet.count == ITEM_COUNT then
                return true
            end
            return nil
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                error(string.format(
                    "노획 MP failed: expected %d got %d",
                    state.expected_mp, caster:mp()))
            end
            return true
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "LOOT SPELL TEST STARTED")

    if spell_runner.run_cases(CASES, caster, nil) == false then
        return false
    end

    log("debug", "LOOT SPELL TEST COMPLETED")
    return true
end

return M
