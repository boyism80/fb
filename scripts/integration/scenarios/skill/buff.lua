local spell_runner = require("integration.lib.spell_runner")

local M = {}

local resp = require("integration.response")
local protocol = require("integration.protocol")
local skill = require("integration.lib.skill")

local CASES = {
    {
        name = "혼마술",
        response = resp.message,
        cast_type = "TARGET",
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.buff_target = target
            state.expected_mp = caster:mp() - 40
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "혼마술") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "혼마술"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "중독",
        response = resp.message,
        cast_type = "TARGET",
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.buff_target = target
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "중독") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "중독"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "절망",
        response = resp.message,
        cast_type = "TARGET",
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.buff_target = target
            state.expected_mp = caster:mp() - 80
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "절망") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "절망"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "저주",
        response = resp.message,
        cast_type = "TARGET",
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.buff_target = target
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "저주") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "저주"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "마비",
        response = resp.message,
        cast_type = "TARGET",
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.buff_target = target
            state.expected_mp = caster:mp() - 80
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "마비") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "마비"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "노도성황",
        response = resp.message,
        cast_type = "TARGET",
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.buff_target = target
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "노도성황") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "노도성황"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "투명",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp()
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "투명") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "투명"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "신의축복",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "신의축복") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "신의축복"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "백호령",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "백호령") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "백호령"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "진백호령",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "진백호령") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "진백호령"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "측면공격",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 90
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "측면공격") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "측면공격"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "후면공격",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 90
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "후면공격") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "후면공격"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "차폐",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 100
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "차폐") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "차폐"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "전혈",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "전혈") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "전혈"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "운기",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "운기") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "운기"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "심안투영",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "심안투영") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "심안투영"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "파력무참",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp()
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "파력무참") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "파력무참"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "신령지익",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp()
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "신령지익") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "신령지익"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "이중공격",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 60
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "이중공격") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "이중공격"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "삼중공격",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 60
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "삼중공격") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "삼중공격"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "사중공격",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 150
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "사중공격") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "사중공격"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "오중공격",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 200
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "오중공격") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "오중공격"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "분신",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp()
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "분신") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "분신"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "누리의힘",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "누리의힘") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "누리의힘"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "금강불체",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            state.buff_target = caster
            state.expected_mp = caster:mp() - 600
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "금강불체") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "금강불체"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "무장",
        response = resp.message,
        cast_type = "TARGET",
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.buff_target = target
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "무장") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "무장"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "보호",
        response = resp.message,
        cast_type = "TARGET",
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.buff_target = target
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "보호") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "보호"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "망각",
        skip = true,
    },
    {
        name = "대지의힘",
        response = resp.message,
        cast_type = "TARGET",
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            state.buff_target = target
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet)
            if packet.text == nil then return nil end
            if skill.is_cast_ready(packet.text, "대지의힘") then return true end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then return false end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "대지의힘"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
}

function M.run(ctx, bot_index, target_index)
    target_index = target_index or 5
    local caster = ctx:bot(bot_index)
    local target = ctx:bot(target_index)
    log("debug", "BUFF/DEBUFF SPELL TEST STARTED")
    caster:setup_bot_stats(100000, 100000)
    target:setup_bot_stats(100000, 100000)
    if spell_runner.run_cases(CASES, caster, target) == false then
        return false
    end
    log("debug", "BUFF/DEBUFF SPELL TEST COMPLETED")
    return true
end
return M
