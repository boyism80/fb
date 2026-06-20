local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local DISGUISE_SPELLS = {
    { spell_name = "경수", mp_cost = 30, available_mobs = {"토끼", "다람쥐"} },
    { spell_name = "맹수", mp_cost = 30, available_mobs = {"곰", "호랑이"} },
    { spell_name = "야수", mp_cost = 30, available_mobs = {"여우", "늑대"} },
    { spell_name = "금수", mp_cost = 30, available_mobs = {"고양이", "새끼돼지", "돼지", "강아지", "누렁이"} },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "DISGUISE SPELL TEST STARTED")

    local names = {}
    for _, spell in ipairs(DISGUISE_SPELLS) do
        table.insert(names, spell.spell_name)
    end
    caster:learn_spells(names)

    local spell_slot = 0
    for _, spell in ipairs(DISGUISE_SPELLS) do
        for _, mob_name in ipairs(spell.available_mobs) do
            caster:set_current_hp_mp(10000, 1000)

            local before_mp = caster:mp()
            local expected_mp = before_mp - spell.mp_cost

            log("debug", spell.spell_name .. " - " .. mob_name)
            caster:request(
                resp.update_internal,
                protocol.spell_cast("INPUT", spell_slot, mob_name, 0, {0, 0}),
                function(packet)
                    return packet.ch_mp == expected_mp
                end)

            caster:remove_buffs()
        end
        spell_slot = spell_slot + 1
    end

    log("debug", "DISGUISE SPELL TEST COMPLETED")
    return true
end

return M
