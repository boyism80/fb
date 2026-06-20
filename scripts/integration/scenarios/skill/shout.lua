local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local SHOUT_SPELLS = {
    { spell_name = "사자후전사", mp_cost = 0, test_message = "전사 사자후 테스트 메시지입니다!", message_type = "SHOUT" },
    { spell_name = "사자후도사", mp_cost = 0, test_message = "도사 사자후 테스트 메시지입니다!", message_type = "SHOUT" },
    { spell_name = "사자후술사", mp_cost = 0, test_message = "술사 사자후 테스트 메시지입니다!", message_type = "SHOUT" },
    { spell_name = "사자후도적", mp_cost = 0, test_message = "도적 사자후 테스트 메시지입니다!", message_type = "SHOUT" },
    { spell_name = "세계후",     mp_cost = 0, test_message = "세계후 테스트 메시지입니다!",      message_type = "WORLD" },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "SHOUT SPELL TEST STARTED")

    local names = {}
    for _, spell in ipairs(SHOUT_SPELLS) do
        table.insert(names, spell.spell_name)
    end
    caster:learn_spells(names)

    local spell_slot = 0
    for _, spell in ipairs(SHOUT_SPELLS) do
        caster:set_current_hp_mp(10000, 1000)

        local before_mp = caster:mp()
        local expected_mp = before_mp - spell.mp_cost

        log("debug", "Testing " .. spell.spell_name)
        caster:request(
            resp.message,
            protocol.spell_cast("INPUT", spell_slot, spell.test_message, 0, {0, 0}),
            function(packet)
                return packet.type == spell.message_type
            end)

        local after_mp = caster:mp()
        if after_mp ~= expected_mp then
            error(string.format(
                "%s MP consumption verification failed: expected %d, got %d",
                spell.spell_name, expected_mp, after_mp))
        end

        spell_slot = spell_slot + 1
    end

    log("debug", "SHOUT SPELL TEST COMPLETED")
    return true
end

return M
