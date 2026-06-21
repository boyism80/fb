local resp         = require("integration.response")
local skill        = require("integration.lib.skill")
local spell_runner = require("integration.lib.spell_runner")

local M = {}

local CASES = {
    {
        name = "어검술",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            state.caster_pos = caster:position()
            caster:spawn_monsters_relative("다람쥐", {{0, 1}, {-1, 2}, {0, 2}, {1, 2}})
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
        post = function(caster, _, state)
            caster:move_back(state.caster_pos, skill.DEFAULT_INTERVAL)
            return true
        end,
    },
    {
        name = "극'백호참",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            state.caster_pos = caster:position()
            caster:spawn_monsters_relative("다람쥐", {{0, 1}, {-1, 2}, {0, 2}, {1, 2}, {-2, 3}, {-1, 3}, {0, 3}, {1, 3}, {2, 3}})
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            state.expected_hp = hp - math.floor(hp / 2)
            state.expected_mp = mp - 50
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            return true
        end,
        post = function(caster, _, state)
            caster:move_back(state.caster_pos, skill.DEFAULT_INTERVAL)
            return true
        end,
    },
    {
        name = "파천검무",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            state.caster_pos = caster:position()
            caster:spawn_monsters_relative("다람쥐", {{0, 1}, {0, 2}, {0, 3}, {0, 4}})
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            local pos = caster:position()
            state.expected_hp = hp - math.floor(hp / 2)
            state.expected_mp = mp - 180
            state.expected_position = {pos[1], pos[2] + 5}
        end,
        condition = function(packet, caster, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            if skill.positions_equal(caster:position(), state.expected_position) == false then
                return nil
            end
            return true
        end,
        post = function(caster, _, state)
            caster:move_back(state.caster_pos, skill.DEFAULT_INTERVAL)
            return true
        end,
    },
    {
        name = "초혼비무",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            state.caster_pos = caster:position()
            caster:spawn_monsters_relative("다람쥐", {{0, 4}})
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            state.expected_hp = hp - math.floor(hp * 2 / 3)
            state.expected_mp = mp - 140
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            return true
        end,
        post = function(caster, _, state)
            caster:move_back(state.caster_pos, skill.DEFAULT_INTERVAL)
            return true
        end,
    },
    {
        name = "쇄혼비무",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            state.caster_pos = caster:position()
            caster:spawn_monsters_relative("다람쥐", {{0, 4}})
            caster:set_current_hp_mp(1000, 1000)
            local hp = caster:hp()
            local mp = caster:mp()
            local pos = caster:position()
            state.expected_hp = hp - math.floor(hp * 2 / 3)
            state.expected_mp = mp - 140
            state.expected_position = {pos[1], pos[2] + 3}
        end,
        condition = function(packet, caster, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            if skill.positions_equal(caster:position(), state.expected_position) == false then
                return nil
            end
            return true
        end,
        post = function(caster, _, state)
            caster:move_back(state.caster_pos, skill.DEFAULT_INTERVAL)
            return true
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "MULTI-TARGET ATTACK_CAST SPELL TEST STARTED")
    caster:setup_bot_stats(100000, 100000)
    if spell_runner.run_cases(CASES, caster, nil) == false then
        return false
    end
    log("debug", "MULTI-TARGET ATTACK_CAST SPELL TEST COMPLETED")
    return true
end
return M
