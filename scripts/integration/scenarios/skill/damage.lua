local spell_runner = require("integration.lib.spell_runner")

local M = {}

local resp = require("integration.response")
local skill = require("integration.lib.skill")

local CASES = {
    {
        name = "뢰진주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 120
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "화염주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 120
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "백열주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 120
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "자무주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 120
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "뢰격주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 180
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "화영열주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 180
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "백령주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 180
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "자영무주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 180
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "뢰격참주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 250
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "화열참주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 250
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "백열참주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 250
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "자천무주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 250
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "진뢰격참주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 330
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "진화열참주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 330
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "진백열참주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 330
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "진자천무주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 330
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "극진뢰격참주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 360
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "극진화열참주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 360
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "극진백열참주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 360
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "극진자천무주",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 360
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "흡성대법",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp() + 15
            state.expected_mp = caster:mp() - 60
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "백열장",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 80
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "헬파이어",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(_, _, state) return state.mob.oid end,
        position = function(_, _, state) return state.mob.position end,
        pre = function(caster, _, state)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = 0
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "DAMAGE SPELL TEST STARTED")
    caster:setup_bot_stats(100000, 100000)
    if spell_runner.run_cases(CASES, caster, nil) == false then
        return false
    end
    log("debug", "DAMAGE SPELL TEST COMPLETED")
    return true
end
return M
