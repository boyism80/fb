local spell_runner = require("integration.lib.spell_runner")

local M = {}

local resp = require("integration.response")
local skill = require("integration.lib.skill")

local CASES = {
    {
        name = "건곤대나이",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            state.expected_hp = hp - math.floor(hp * 2 / 3)
            state.expected_mp = mp - 40
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            return true
        end,
    },
    {
        name = "동귀어진",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            state.expected_hp = hp - (hp - 10)
            state.expected_mp = mp - 10
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            return true
        end,
    },
    {
        name = "백호검무",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            state.expected_hp = hp - math.floor(hp / 2)
            state.expected_mp = mp - 60
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            return true
        end,
    },
    {
        name = "백호참",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            state.expected_hp = hp - math.floor(hp / 2)
            state.expected_mp = mp - 60
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            return true
        end,
    },
    {
        name = "필살검무",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            state.expected_hp = hp - math.floor(hp / 2)
            state.expected_mp = mp - math.max(mp, 60)
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            return true
        end,
    },
    {
        name = "무형검",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            state.expected_hp = hp - math.floor(hp / 2)
            state.expected_mp = mp - 180
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            return true
        end,
    },
    {
        name = "이기어검",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            state.expected_hp = hp - math.floor(hp / 2)
            state.expected_mp = mp - 180
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            return true
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "ATTACK_CAST SPELL TEST STARTED")
    caster:setup_bot_stats(100000, 100000)
    if spell_runner.run_cases(CASES, caster, nil) == false then
        return false
    end
    log("debug", "ATTACK_CAST SPELL TEST COMPLETED")
    return true
end
return M
