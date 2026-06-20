local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local HEALING_SPELLS = {
    { name = "누리의기원", type = "NORMAL", expected_hp_gain = 50,    expected_mp_cost = 30   },
    { name = "하늘의기원", type = "NORMAL", expected_hp_gain = 200,   expected_mp_cost = 120  },
    { name = "대지의기원", type = "TARGET", expected_hp_gain = 50,    expected_mp_cost = 50   },
    { name = "동해의기원", type = "TARGET", expected_hp_gain = 100,   expected_mp_cost = 30   },
    { name = "바다의기원", type = "NORMAL", expected_hp_gain = 100,   expected_mp_cost = 50   },
    { name = "천공의기원", type = "TARGET", expected_hp_gain = 200,   expected_mp_cost = 100  },
    { name = "구름의기원", type = "TARGET", expected_hp_gain = 500,   expected_mp_cost = 120  },
    { name = "태양의기원", type = "TARGET", expected_hp_gain = 1000,  expected_mp_cost = 240  },
    { name = "생명의기원", type = "TARGET", expected_hp_gain = 5000,  expected_mp_cost = 300  },
    { name = "현자의기원", type = "TARGET", expected_hp_gain = 5000,  expected_mp_cost = 1000 },
    { name = "신령의기원", type = "TARGET", expected_hp_gain = 10000, expected_mp_cost = 1000 },
    { name = "봉황의기원", type = "TARGET", expected_hp_gain = 30000, expected_mp_cost = 10000},
    { name = "천공의희원", type = "TARGET", expected_hp_gain = 200,   expected_mp_cost = 200  },
    { name = "구름의희원", type = "TARGET", expected_hp_gain = 500,   expected_mp_cost = 480  },
    { name = "태양의희원", type = "TARGET", expected_hp_gain = 1000,  expected_mp_cost = 960  },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "HEALING SPELL TEST STARTED")

    caster:setup_bot_stats(100000, 100000, 50)
    caster:setup_bot_stats(100000, 100000, 50)

    caster:learn_spells(skill.spell_names(HEALING_SPELLS))
    caster:base_hp(100000)
    caster:hp(50)

    local spell_slot = 0
    for _, spell in ipairs(HEALING_SPELLS) do
        if spell.type == "NORMAL" then
            local before_hp = caster:hp()
            local expected_hp = before_hp + spell.expected_hp_gain

            log("debug", "Testing " .. spell.name)
            skill.request_update_internal(
                caster, resp, protocol,
                spell.type, spell_slot, "", 0, {0, 0},
                expected_hp, nil)
        else
            local before_caster_hp = caster:hp()
            local before_caster_mp = caster:mp()
            local pos = caster:position()

            log("debug", "Testing " .. spell.name)
            skill.request_update_internal(
                caster, resp, protocol,
                spell.type, spell_slot, "", caster:oid(), pos,
                before_caster_hp + spell.expected_hp_gain,
                before_caster_mp - spell.expected_mp_cost)
        end
        spell_slot = spell_slot + 1
    end

    -- Dynamic spell: 백호의희원
    local dynamic_slot = caster:learn_spell("백호의희원")
    if dynamic_slot == 0xFF then
        return false
    end

    caster:mp(100)
    caster:hp(50)

    local current_mp = caster:mp()
    local theoretical_hp_gain = current_mp * 2
    local expected_mp_cost = math.floor(current_mp / 2)
    local caster_max_hp = caster:base_hp()
    local caster_current_hp = caster:hp()
    local caster_recoverable_hp = caster_max_hp - caster_current_hp
    local actual_expected_hp_gain = math.min(theoretical_hp_gain, caster_recoverable_hp)

    if theoretical_hp_gain > caster_recoverable_hp then
        local new_caster_hp = caster_max_hp - theoretical_hp_gain
        if new_caster_hp < 1 then
            new_caster_hp = 1
        end
        caster:hp(new_caster_hp)
        actual_expected_hp_gain = theoretical_hp_gain
    end

    local before_caster_hp = caster:hp()
    local before_caster_mp = caster:mp()
    local pos = caster:position()

    skill.request_update_internal(
        caster, resp, protocol,
        "TARGET", spell_slot, "", caster:oid(), pos,
        before_caster_hp + actual_expected_hp_gain,
        before_caster_mp - expected_mp_cost)

    log("debug", "All healing spell tests completed successfully!")
    return true
end

return M
