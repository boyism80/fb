local spell_runner = require("integration.lib.spell_runner")
local lib = require("integration.lib")

local M = {}

local resp = require("integration.response")

local SINGLE_TARGET_SPELLS = {
    { name = "뢰진주", mp = 120 },
    { name = "화염주", mp = 120 },
    { name = "백열주", mp = 120 },
    { name = "자무주", mp = 120 },
    { name = "뢰격주", mp = 180 },
    { name = "화영열주", mp = 180 },
    { name = "백령주", mp = 180 },
    { name = "자영무주", mp = 180 },
    { name = "뢰격참주", mp = 250 },
    { name = "화열참주", mp = 250 },
    { name = "백열참주", mp = 250 },
    { name = "자천무주", mp = 250 },
    { name = "진뢰격참주", mp = 330 },
    { name = "진화열참주", mp = 330 },
    { name = "진백열참주", mp = 330 },
    { name = "진자천무주", mp = 330 },
    { name = "극진뢰격참주", mp = 360 },
    { name = "극진화열참주", mp = 360 },
    { name = "극진백열참주", mp = 360 },
    { name = "극진자천무주", mp = 360 },
}

local function build_single_target_cases()
    local cases = {}
    for _, spell in ipairs(SINGLE_TARGET_SPELLS) do
        table.insert(cases, {
            name = spell.name,
            response = resp.update_internal,
            cast_type = "TARGET",
            pre = function(caster, target, state)
                caster:setup_bot_stats(100000, 100000)
                target:setup_bot_stats(100000, 100000)
                caster:set_current_hp_mp(50, 100000)
                target:set_current_hp_mp(target:base_hp(), target:mp())
                state.expected_mp = caster:mp() - spell.mp
            end,
            condition = function(packet, _, _, state)
                if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                    return nil
                end
                return true
            end,
        })
    end
    return cases
end

local MOB_CASES = {
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
        condition = function(packet, _, _, state)
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
        condition = function(packet, _, _, state)
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
        condition = function(packet, _, _, state)
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

function M.run(ctx, bot_index, target_index)
    target_index = target_index or lib.formation.skill_pair_target(bot_index)
    if target_index == nil then
        log("debug", "DAMAGE SPELL TEST requires a target bot")
        return false
    end
    local caster = ctx:bot(bot_index)
    local target = ctx:bot(target_index)
    log("debug", "DAMAGE SPELL TEST STARTED")

    if lib.option.disable_pk_protect(caster) == false then
        log("fatal", "DAMAGE SPELL TEST: failed to disable PK_PROTECT for caster")
        return false
    end

    local single_target_cases = build_single_target_cases()
    if spell_runner.run_cases(single_target_cases, caster, target) == false then
        return false
    end

    if spell_runner.run_cases(MOB_CASES, caster, target, { start_slot = #single_target_cases }) == false then
        return false
    end

    log("debug", "DAMAGE SPELL TEST COMPLETED")
    return true
end

return M
