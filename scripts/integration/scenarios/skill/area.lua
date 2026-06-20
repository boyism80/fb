local spell_runner = require("integration.lib.spell_runner")

local M = {}

local MAP = "낙랑의방"

local TEST_POSITIONS = {
    [0] = { x = 6, y = 9 },
    [1] = { x = 5, y = 9 },
    [2] = { x = 6, y = 8 },
    [3] = { x = 6, y = 10 },
    [4] = { x = 7, y = 9 },
}

local resp = require("integration.response")
local skill = require("integration.lib.skill")

local function build_cases(ctx)
    return {
        {
            name = "분혼경천",
            response = resp.update_internal,
            cast_type = "NORMAL",
            pre = function(caster, _, state)
                caster:spawn_monsters_bulk("다람쥐", 5)
                caster:set_current_hp_mp(1000, 1000)
                for i = 1, 4 do
                    ctx:bot(i):set_current_hp_mp(10000, 10000)
                end
                local hp = caster:hp()
                local mp = caster:mp()
                state.expected_hp = hp - math.floor(hp / 2)
                state.expected_mp = mp - 200
            end,
            condition = function(packet, _, _, state)
                if packet.ch_hp ~= state.expected_hp then return nil end
                if packet.ch_mp ~= state.expected_mp then return nil end
                return true
            end,
        },
        {
            name = "혈겁만파",
            response = resp.update_internal,
            cast_type = "NORMAL",
            pre = function(caster, _, state)
                caster:spawn_monsters_bulk("다람쥐", 5)
                caster:set_current_hp_mp(1000, 1000)
                for i = 1, 4 do
                    ctx:bot(i):set_current_hp_mp(10000, 10000)
                end
                local hp = caster:hp()
                local mp = caster:mp()
                state.expected_hp = hp - math.floor(hp / 2)
                state.expected_mp = mp - 600
            end,
            condition = function(packet, _, _, state)
                if packet.ch_hp ~= state.expected_hp then return nil end
                if packet.ch_mp ~= state.expected_mp then return nil end
                return true
            end,
        },
        {
            name = "폭류유성",
            response = resp.update_internal,
            cast_type = "NORMAL",
            pre = function(caster, _, state)
                caster:spawn_monsters_bulk("다람쥐", 5)
                caster:set_current_hp_mp(1000, 1000)
                for i = 1, 4 do
                    ctx:bot(i):set_current_hp_mp(10000, 10000)
                end
                local hp = caster:hp()
                local mp = caster:mp()
                state.expected_hp = math.floor(hp / 2)
                state.expected_mp = mp - math.max(30, math.floor(mp / 2))
            end,
            condition = function(packet, _, _, state)
                if packet.ch_hp ~= state.expected_hp then return nil end
                if packet.ch_mp ~= state.expected_mp then return nil end
                return true
            end,
        },
        {
            name = "포효검황",
            response = resp.update_internal,
            cast_type = "NORMAL",
            pre = function(caster, _, state)
                caster:spawn_monsters_bulk("다람쥐", 5)
                caster:set_current_hp_mp(1000, 1000)
                for i = 1, 4 do
                    ctx:bot(i):set_current_hp_mp(10000, 10000)
                end
                local hp = caster:hp()
                local mp = caster:mp()
                state.expected_hp = hp - math.floor(hp / 2)
                state.expected_mp = mp - 600
            end,
            condition = function(packet, _, _, state)
                if packet.ch_hp ~= state.expected_hp then return nil end
                if packet.ch_mp ~= state.expected_mp then return nil end
                return true
            end,
        },
        {
            name = "지폭지술",
            response = resp.update_internal,
            cast_type = "NORMAL",
            pre = function(caster, _, state)
                caster:spawn_monsters_bulk("다람쥐", 5)
                caster:set_current_hp_mp(1000, 1000)
                for i = 1, 4 do
                    ctx:bot(i):set_current_hp_mp(10000, 10000)
                end
                local hp = caster:hp()
                local mp = caster:mp()
                state.expected_hp = hp
                state.expected_mp = mp - math.max(mp, 30)
            end,
            condition = function(packet, _, _, state)
                if packet.ch_hp ~= state.expected_hp then return nil end
                if packet.ch_mp ~= state.expected_mp then return nil end
                return true
            end,
        },
    }
end

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "AREA DAMAGE SPELL TEST STARTED")
    for i = 0, ctx:bot_count() - 1 do
        ctx:bot(i):setup_bot_stats(100000, 100000)
    end
    local cases = build_cases(ctx)
    for i = 0, 4 do
        local pos = TEST_POSITIONS[i]
        ctx:bot(i):map_move(MAP, pos.x, pos.y)
    end
    caster:direction("BOTTOM")
    ctx:bot(1):direction("BOTTOM")
    ctx:bot(2):direction("BOTTOM")
    ctx:bot(3):direction("BOTTOM")
    ctx:bot(4):direction("BOTTOM")
    if spell_runner.run_cases(cases, caster, nil) == false then
        return false
    end
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
