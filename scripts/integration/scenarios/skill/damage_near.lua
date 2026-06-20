local skill = require("integration.lib.skill")
local spell_runner = require("integration.lib.spell_runner")

local M = {}

local resp = require("integration.response")
local skill = require("integration.lib.skill")

local CASES = {
    {
        name = "뢰진주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "화염주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "백열주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "자무주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "뢰격주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "화영열주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "백령주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "자영무주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "뢰격참주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "화열참주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "백열참주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "자천무주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
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
        name = "진뢰격참주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 400
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
        name = "진화열참주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 400
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
        name = "진백열참주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 400
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
        name = "진자천무주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 400
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
        name = "극진뢰격참주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 470
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
        name = "극진화열참주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 470
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
        name = "극진백열참주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 470
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
        name = "극진자천무주'첨",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:spawn_monsters_bulk("다람쥐", 1)
            caster:set_current_hp_mp(50, 100000)
            state.expected_hp = caster:hp()
            state.expected_mp = caster:mp() - 470
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
    log("debug", "NEAR DAMAGE SPELL TEST STARTED")
    caster:move("LEFT")
    ctx:sleep(skill.DEFAULT_INTERVAL)
    caster:direction("BOTTOM")
    caster:setup_bot_stats(100000, 100000)
    if spell_runner.run_cases(CASES, caster, nil) == false then
        return false
    end
    ctx:sleep(skill.DEFAULT_INTERVAL)
    caster:move("RIGHT")
    caster:direction("BOTTOM")
    log("debug", "NEAR DAMAGE SPELL TEST COMPLETED")
    return true
end
return M
