local spell_runner = require("integration.lib.spell_runner")

local M = {}

local resp = require("integration.response")
local skill = require("integration.lib.skill")

local CASES = {
    {
        name = "삼매진화",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 2)
            caster:spawn_monsters_relative("다람쥐", {{0, 1}, {-1, 2}, {1, 2}, {0, 3}})
            caster:set_current_hp_mp(1000, 1000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - caster:mp()
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then return nil end
            if packet.ch_mp ~= state.expected_mp then return nil end
            return true
        end,
    },
    {
        name = "지진",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 2)
            caster:spawn_monsters_relative("다람쥐", {{0, 1}, {-1, 2}, {1, 2}, {0, 3}})
            caster:set_current_hp_mp(1000, 1000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 580
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
    log("debug", "NEAR TARGET DAMAGE SPELL TEST STARTED")
    caster:setup_bot_stats(100000, 100000)
    if spell_runner.run_cases(CASES, caster, nil) == false then
        return false
    end
    log("debug", "NEAR TARGET DAMAGE SPELL TEST COMPLETED")
    return true
end
return M
