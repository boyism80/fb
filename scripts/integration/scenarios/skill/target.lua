local spell_runner = require("integration.lib.spell_runner")

local M = {}

local resp = require("integration.response")
local protocol = require("integration.protocol")
local skill = require("integration.lib.skill")

local CASES = {
    {
        name = "공력주입",
        response = resp.update_internal,
        cast_type = "TARGET",
        pre = function(caster, target, state)
            caster:setup_bot_stats(100000, 100000)
            target:setup_bot_stats(100000, 100000)
            target:set_current_hp_mp(target:hp(), 0)
            caster:set_current_hp_mp(caster:hp(), caster:base_mp())
            state.injected = caster:mp()
            state.target_before_mp = target:mp()
        end,
        condition = function(packet)
            if packet.ch_mp == 0 then
                return true
            end
            return nil
        end,
        post = function(caster, target, state, packet)
            if packet.ch_mp ~= 0 then
                return false
            end
            local expected = math.min(target:base_mp(), state.target_before_mp + state.injected)
            local result = target:request(resp.update_internal, protocol.self_info(), function(p)
                return p.ch_mp == expected
            end)
            return result ~= false and result ~= nil
        end,
    },
    {
        name = "대지의빛",
        response = resp.message,
        cast_type = "TARGET",
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.expected_mp = caster:mp() - 30
        end,
condition = function(packet)
    if packet.text == nil then
        return nil
    end
    if skill.is_cast_ready(packet.text, "대지의빛") then
        return true
    end
    return false
end,
        post = function(caster, _, state)
            return caster:mp() == state.expected_mp
        end,
    },
    {
        name = "부활",
        response = resp.message,
        cast_type = "TARGET",
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.expected_mp = caster:mp() - 300
        end,
condition = function(packet)
    if packet.text == nil then
        return nil
    end
    if skill.is_cast_ready(packet.text, "부활") then
        return true
    end
    return false
end,
        post = function(caster, _, state)
            return caster:mp() == state.expected_mp
        end,
    },
    {
        name = "시력회복",
        response = resp.message,
        cast_type = "TARGET",
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.expected_mp = caster:mp() - 40
        end,
condition = function(packet)
    if packet.text == nil then
        return nil
    end
    if skill.is_cast_ready(packet.text, "시력회복") then
        return true
    end
    return false
end,
        post = function(caster, _, state)
            return caster:mp() == state.expected_mp
        end,
    },
    {
        name = "퇴마주",
        response = resp.message,
        cast_type = "TARGET",
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.expected_mp = caster:mp() - 30
        end,
condition = function(packet)
    if packet.text == nil then
        return nil
    end
    if skill.is_cast_ready(packet.text, "퇴마주") then
        return true
    end
    return false
end,
        post = function(caster, _, state)
            return caster:mp() == state.expected_mp
        end,
    },
    {
        name = "파혼술",
        response = resp.message,
        cast_type = "TARGET",
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.expected_mp = caster:mp() - 30
        end,
condition = function(packet)
    if packet.text == nil then
        return nil
    end
    if skill.is_cast_ready(packet.text, "파혼술") then
        return true
    end
    return false
end,
        post = function(caster, _, state)
            return caster:mp() == state.expected_mp
        end,
    },
    {
        name = "해독",
        response = resp.message,
        cast_type = "TARGET",
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.expected_mp = caster:mp() - 30
        end,
condition = function(packet)
    if packet.text == nil then
        return nil
    end
    if skill.is_cast_ready(packet.text, "해독") then
        return true
    end
    return false
end,
        post = function(caster, _, state)
            return caster:mp() == state.expected_mp
        end,
    },
    {
        name = "활력",
        response = resp.message,
        cast_type = "TARGET",
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.expected_mp = caster:mp() - 30
        end,
condition = function(packet)
    if packet.text == nil then
        return nil
    end
    if skill.is_cast_ready(packet.text, "활력") then
        return true
    end
    return false
end,
        post = function(caster, _, state)
            return caster:mp() == state.expected_mp
        end,
    },
}

M.TARGET_BOT_INDEX = 7

function M.run(ctx, bot_index, target_index)
    target_index = target_index or M.TARGET_BOT_INDEX
    local caster = ctx:bot(bot_index)
    local target = ctx:bot(target_index)
    log("debug", "TARGET SPELL TEST STARTED")
    if spell_runner.run_cases(CASES, caster, target) == false then
        return false
    end
    log("debug", "TARGET SPELL TEST COMPLETED")
    return true
end
return M
