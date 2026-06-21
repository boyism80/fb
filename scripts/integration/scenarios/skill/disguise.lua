local spell_runner = require("integration.lib.spell_runner")

local M = {}

local resp = require("integration.response")

local function disguise_condition(packet, _, _, state)
    if packet.ch_mp == state.expected_mp then return true end
    return nil
end

local function disguise_pre(caster, _, state)
    caster:set_current_hp_mp(10000, 1000)
    state.expected_mp = caster:mp() - 30
end

local function disguise_post(caster)
    caster:remove_buffs()
    return true
end

local GROUPS = {
    {
        spell = "경수",
        response = resp.update_internal,
        cast_type = "INPUT",
        pre = disguise_pre,
        condition = disguise_condition,
        post = disguise_post,
        variants = {
            "토끼",
            "다람쥐",
        },
    },
    {
        spell = "맹수",
        response = resp.update_internal,
        cast_type = "INPUT",
        pre = disguise_pre,
        condition = disguise_condition,
        post = disguise_post,
        variants = {
            "곰",
            "호랑이",
        },
    },
    {
        spell = "야수",
        response = resp.update_internal,
        cast_type = "INPUT",
        pre = disguise_pre,
        condition = disguise_condition,
        post = disguise_post,
        variants = {
            "여우",
            "늑대",
        },
    },
    {
        spell = "금수",
        response = resp.update_internal,
        cast_type = "INPUT",
        pre = disguise_pre,
        condition = disguise_condition,
        post = disguise_post,
        variants = {
            "고양이",
            "새끼돼지",
            "돼지",
            "강아지",
            "누렁이",
        },
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "DISGUISE SPELL TEST STARTED")
    if spell_runner.run_spell_groups(GROUPS, caster, nil) == false then
        return false
    end
    log("debug", "DISGUISE SPELL TEST COMPLETED")
    return true
end

return M
