local spell_runner = require("integration.lib.spell_runner")
local lib = require("integration.lib")

local M = {}

local resp = require("integration.response")
local protocol = require("integration.protocol")
local skill = require("integration.lib.skill")

local CASES = {
    {
        name = "공력주입",
        pre = function(caster, target, state)
            caster:setup_bot_stats(100000, 100000)
            target:setup_bot_stats(100000, 100000)
            target:set_current_hp_mp(target:hp(), 0)
            caster:set_current_hp_mp(caster:hp(), caster:base_mp())
            state.injected = caster:mp()
            state.target_before_mp = target:mp()
        end,
        cast = function(caster, target, slot, state)
            local expected = math.min(target:base_mp(), state.target_before_mp + state.injected)
            local target_result = caster:request_on(
                target,
                resp.update_internal,
                protocol.spell_cast("TARGET", slot, "", target:oid(), target:position()),
                function(p)
                    return p.ch_mp == expected
                end)
            if target_result == false or target_result == nil then
                return false
            end

            caster:sleep(1000)

            if caster:mp() ~= 0 then
                return false
            end
            state.packet = target_result
            return true
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
            state.target_name = target:name()
        end,
        condition = function(packet, _, _, state)
            if packet.text == nil then
                return nil
            end
            if packet.type ~= "POPUP" then
                return nil
            end
            if packet.text:find(state.target_name, 1, true) == nil then
                return nil
            end
            if packet.text:find("레벨", 1, true) == nil then
                return nil
            end
            if packet.text:find("힘", 1, true) == nil then
                return nil
            end
            if packet.text:find("민첩성", 1, true) == nil then
                return nil
            end
            if packet.text:find("지능", 1, true) == nil then
                return nil
            end
            return true
        end,
        post = function(caster, _, state)
            return caster:mp() == state.expected_mp
        end,
    },
    {
        name = "부활",
        response = resp.update_external_detailed,
        cast_type = "TARGET",
        pre = function(caster, target, state)
            caster:setup_bot_stats(100000, 100000)
            target:setup_bot_stats(100000, 100000)
            caster:set_current_hp_mp(caster:hp(), 1000)
            target:set_current_hp_mp(50, target:mp())

            state.hell_slot = caster:learn_spell("헬파이어")
            if state.hell_slot == 0xFF then
                return false
            end

            local pos = target:position()
            local packet = caster:request(
                resp.update_external_detailed,
                protocol.spell_cast("TARGET", state.hell_slot, "", target:oid(), pos),
                function(pkt)
                    return pkt.oid == target:oid() and pkt.state == "GHOST"
                end)
            if packet == false or packet == nil then
                return false
            end

            -- Hellfire consumes all current MP; 부활 requires 300 MP.
            caster:set_current_hp_mp(caster:hp(), 1000)
            return true
        end,
        condition = function(packet, _, target)
            if packet.oid ~= target:oid() then
                return nil
            end
            if packet.state == "NORMAL" then
                return true
            end
            return nil
        end,
        post = function(caster, target, state, packet)
            if packet.oid ~= target:oid() or packet.state ~= "NORMAL" then
                return false
            end
            if state.hell_slot ~= nil and state.hell_slot ~= 0xFF then
                caster:chat("/마법지우기 " .. state.hell_slot)
            end
            return true
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

function M.run(ctx, bot_index, target_index)
    target_index = target_index or lib.formation.skill_pair_target(bot_index)
    if target_index == nil then
        log("debug", "TARGET SPELL TEST requires a target bot")
        return false
    end
    local caster = ctx:bot(bot_index)
    local target = ctx:bot(target_index)
    log("debug", "TARGET SPELL TEST STARTED")
    if lib.option.disable_pk_protect(caster) == false then
        log("fatal", "TARGET SPELL TEST: failed to disable PK_PROTECT for caster")
        return false
    end
    if spell_runner.run_cases(CASES, caster, target) == false then
        return false
    end
    log("debug", "TARGET SPELL TEST COMPLETED")
    return true
end
return M
