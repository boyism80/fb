local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local MAP = "낙랑의방"

-- Positions after the original step-by-step move setup (from formation line at y=6).
local TEST_POSITIONS = {
    [0] = { x = 6, y = 9 },
    [1] = { x = 5, y = 9 },
    [2] = { x = 6, y = 8 },
    [3] = { x = 6, y = 10 },
    [4] = { x = 7, y = 9 },
}

local AREA_SPELLS = {
    {
        name = "분혼경천",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp / 2)
            return current_hp - hp_cost, current_mp - 200
        end,
    },
    {
        name = "혈겁만파",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp / 2)
            return current_hp - hp_cost, current_mp - 600
        end,
    },
    {
        name = "폭류유성",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_final = math.floor(current_hp / 2)
            local mp_cost = math.max(30, math.floor(current_mp / 2))
            return hp_final, current_mp - mp_cost
        end,
    },
    {
        name = "포효검황",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp / 2)
            return current_hp - hp_cost, current_mp - 600
        end,
    },
    {
        name = "지폭지술",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local mp_cost = math.max(current_mp, 30)
            return current_hp, current_mp - mp_cost
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "AREA DAMAGE SPELL TEST STARTED")

    for i = 0, ctx:bot_count() - 1 do
        ctx:bot(i):setup_bot_stats(100000, 100000)
    end

    caster:learn_spells(skill.spell_names(AREA_SPELLS))

    for i = 0, 4 do
        local pos = TEST_POSITIONS[i]
        ctx:bot(i):map_move(MAP, pos.x, pos.y)
    end

    caster:direction("BOTTOM")
    ctx:bot(1):direction("BOTTOM")
    ctx:bot(2):direction("BOTTOM")
    ctx:bot(3):direction("BOTTOM")
    ctx:bot(4):direction("BOTTOM")

    local spell_slot = 0
    for _, spell in ipairs(AREA_SPELLS) do
        caster:spawn_monsters_bulk("다람쥐", 5)
        caster:set_current_hp_mp(1000, 1000)

        for i = 1, 4 do
            ctx:bot(i):set_current_hp_mp(10000, 10000)
        end

        local expected_caster_hp, expected_caster_mp = spell.calculator(caster)

        log("debug", "Testing " .. spell.name)
        skill.request_update_internal(
            caster, resp, protocol,
            spell.type, spell_slot, "", 0, {0, 0},
            expected_caster_hp, expected_caster_mp)

        spell_slot = spell_slot + 1
    end

    -- Restore formation line (6,6), (7,6), ...
    for i = 0, 4 do
        ctx:bot(i):map_move(MAP, 6 + i, 6)
    end

    ctx:bot(4):direction("BOTTOM")
    ctx:bot(3):direction("BOTTOM")
    ctx:bot(2):direction("BOTTOM")
    ctx:bot(1):direction("BOTTOM")
    caster:direction("BOTTOM")

    log("debug", "AREA DAMAGE SPELL TEST COMPLETED")
    return true
end

return M
