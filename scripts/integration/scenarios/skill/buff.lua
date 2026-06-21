local spell_runner = require("integration.lib.spell_runner")
local lib = require("integration.lib")

local M = {}

local resp = require("integration.response")
local protocol = require("integration.protocol")
local skill = require("integration.lib.skill")

local ATTACK_INTERVAL = 100
local ATTACK_MAX_LOOPS = 300
local MOB_CLEAR_RANGE = 2
local MOB_NAME = "다람쥐"

local function request_phydef(bot)
    local info = bot:request(
        resp.internal_info,
        protocol.self_info(),
        function(_)
            return true
        end)
    if info == false or info == nil or info.phydef == nil then
        return nil
    end
    return info.phydef
end

local function make_phydef_debuff_case(name, mp_cost, phydef_delta, dispel_name, dispel_mp)
    return {
        name = name,
        response = resp.update_internal,
        cast_type = "TARGET",
        clear_target_buffs_on_retry = false,
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            target:remove_buffs()
            state.buff_target = target
            state.expected_mp = caster:mp() - mp_cost
            state.phydef_delta = phydef_delta
            state.dispel_name = dispel_name
            state.dispel_mp = dispel_mp
            state.base_phydef = request_phydef(target)
            if state.base_phydef == nil then
                return false
            end
        end,
        condition = function(packet, _, _, state)
            if packet.ch_mp ~= state.expected_mp then
                return false
            end
            return true
        end,
        post = function(caster, target, state, packet)
            if packet == false or packet.ch_mp ~= state.expected_mp then
                return false
            end
            if caster:mp() ~= state.expected_mp then
                return false
            end

            local debuffed_phydef = request_phydef(target)
            if debuffed_phydef == nil then
                return false
            end
            if debuffed_phydef ~= state.base_phydef + state.phydef_delta then
                return false
            end

            local dispel_slot = caster:learn_spell(state.dispel_name)
            if dispel_slot == 0xFF then
                return false
            end

            state.expected_mp_dispel = caster:mp() - state.dispel_mp
            local dispel_result = caster:request(
                resp.update_internal,
                protocol.spell_cast("TARGET", dispel_slot, "", target:oid(), target:position()),
                function(p)
                    return p.ch_mp == state.expected_mp_dispel
                end)
            if dispel_result == false or dispel_result == nil then
                return false
            end
            if caster:mp() ~= state.expected_mp_dispel then
                return false
            end

            local restored_phydef = request_phydef(target)
            if restored_phydef == nil then
                return false
            end
            if restored_phydef ~= state.base_phydef then
                return false
            end

            state.buff_target:remove_buffs()
            return true
        end,
    }
end

local function restore_target_position(target, state)
    if target ~= nil and state.target_aside == true then
        target:move("LEFT", 1)
        state.target_aside = false
    end
end

local function offset_target_aside(target, state)
    if target == nil then
        return true
    end
    if state.target_aside ~= true then
        target:move("RIGHT", 1)
        state.target_aside = true
    end
    return true
end

local function lure_mob(caster, lure_slot, mob)
    local lure_ok = caster:request(
    resp.action,
    protocol.spell_cast("TARGET", lure_slot, "", mob.oid, mob.position),
    function(packet)
        return packet.oid == caster:oid()
    end)
    return lure_ok ~= false and lure_ok ~= nil
end

local function attack_until_mobs_dead(caster, ctx, mob_oids, label)
    local dead = {}
    
    log("debug", string.format(
    "[%s] attack_until_mobs_dead begin caster_oid=%s targets=%s",
    label,
    tostring(caster:oid()),
    table.concat(mob_oids, ",")))
    
    ctx:hook("die", function(_, _, packet)
        for _, oid in ipairs(mob_oids) do
            if packet.oid == oid then
                dead[oid] = true
                log("debug", string.format("[%s] mob died oid=%s", label, tostring(oid)))
            end
        end
    end)
    
    for loop = 1, ATTACK_MAX_LOOPS do
        local all_dead = true
        local alive_oids = {}
        for _, oid in ipairs(mob_oids) do
            if dead[oid] ~= true then
                all_dead = false
                table.insert(alive_oids, tostring(oid))
            end
        end
        if all_dead then
            ctx:unhook("die")
            log("debug", string.format("[%s] attack_until_mobs_dead ok loops=%d", label, loop))
            return true
        end
        
        if loop == 1 or loop % 50 == 0 then
            log("debug", string.format(
            "[%s] attack loop=%d alive=%s pos=(%d,%d) direction=BOTTOM",
            label,
            loop,
            table.concat(alive_oids, ","),
            caster:position()[1],
            caster:position()[2]))
        end
        
        caster:request(
        resp.action,
        protocol.attack(),
        function(packet)
            return packet.oid == caster:oid() and packet.value == "ATTACK"
        end)
        caster:sleep(ATTACK_INTERVAL)
    end
    
    ctx:unhook("die")
    log("debug", string.format(
    "[%s] attack_until_mobs_dead timeout after %d loops, dead=%s",
    label,
    ATTACK_MAX_LOOPS,
    table.concat(mob_oids, ",")))
    return false
end

local function make_directional_attack_case(name, mob_offsets)
    return {
        name = name,
        response = resp.message,
        cast_type = "NORMAL",
        clear_target_buffs_on_retry = false,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            caster:direction("BOTTOM")
            state.expected_mp = caster:mp() - 90
            
            if state.lure_slot == nil then
                state.lure_slot = caster:learn_spell("유인")
                if state.lure_slot == 0xFF then
                    return false
                end
            end
            
            if offset_target_aside(target, state) == false then
                return false
            end
            
            caster:chat(string.format("/몬스터범위제거 %d", MOB_CLEAR_RANGE))
            return true
        end,
        condition = function(packet)
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, name) then
                return true
            end
            return false
        end,
        post = function(caster, target, state, _, ctx)
            local function finish(ok)
                caster:chat(string.format("/몬스터범위제거 %d", MOB_CLEAR_RANGE))
                caster:chat(string.format("/아이템범위제거 %d", MOB_CLEAR_RANGE))
                restore_target_position(target, state)
                return ok
            end
            
            if ctx == nil then
                return finish(false)
            end
            if caster:mp() ~= state.expected_mp then
                return finish(false)
            end
            
            local buff_result = caster:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == name
            end)
            if buff_result == false or buff_result == nil then
                return finish(false)
            end
            
            local mob_oids = {}
            for _, offset in ipairs(mob_offsets) do
                local mob = caster:spawn_monster_relative(MOB_NAME, offset[1], offset[2])
                if mob == nil or mob.oid == nil then
                    return finish(false)
                end
                if lure_mob(caster, state.lure_slot, mob) == false then
                    return finish(false)
                end
                table.insert(mob_oids, mob.oid)
            end
            
            if attack_until_mobs_dead(caster, ctx, mob_oids, name) == false then
                return finish(false)
            end
            
            caster:remove_buffs()
            return finish(true)
        end,
    }
end

local CASES = {
    make_phydef_debuff_case("혼마술", 40, 50, "파혼술", 30),
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "중독") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "중독"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    {
        name = "절망",
        response = resp.update_cc,
        cast_type = "TARGET",
        clear_target_buffs_on_retry = false,
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 10000)
            caster:remove_buffs()
            state.expected_mp = caster:mp() - 80
        end,
        condition = function(packet)
            if packet.sight == true then
                return true
            end
            if packet.sight == false then
                return false
            end
            return nil
        end,
        post = function(caster, _, state, packet)
            if packet == false or packet.sight ~= true then
                return false
            end
            if caster:mp() ~= state.expected_mp then
                return false
            end
            
            local cc_sight = 0x02
            if (caster:crowd_control() & cc_sight) == 0 then
                return false
            end
            
            local recover_slot = caster:learn_spell("시력회복")
            if recover_slot == 0xFF then
                return false
            end
            
            state.expected_mp_recover = caster:mp() - 40
            local cleared = caster:request(
            resp.update_cc,
            protocol.spell_cast("TARGET", recover_slot, "", caster:oid(), caster:position()),
            function(p)
                if p.sight == false then
                    return true
                end
                if p.sight == true then
                    return false
                end
                return false
            end)
            if cleared == false or cleared == nil then
                return false
            end
            if caster:mp() ~= state.expected_mp_recover then
                return false
            end
            if (caster:crowd_control() & cc_sight) ~= 0 then
                return false
            end
            
            caster:remove_buffs()
            return true
        end,
    },
    make_phydef_debuff_case("저주", 30, 30, "퇴마주", 30),
    {
        name = "마비",
        response = resp.update_internal,
        cast_type = "TARGET",
        clear_target_buffs_on_retry = false,
        oid = function(_, target) return target:oid() end,
        position = function(_, target) return target:position() end,
        pre = function(caster, target, state)
            caster:set_current_hp_mp(10000, 10000)
            target:set_current_hp_mp(10000, 10000)
            target:remove_buffs()
            state.buff_target = target
            state.expected_mp = caster:mp() - 80
            state.original_position = {target:position()[1], target:position()[2]}
        end,
        condition = function(packet, _, _, state)
            if packet.ch_mp ~= state.expected_mp then
                return false
            end
            return true
        end,
        post = function(caster, target, state, packet)
            if packet == false or packet.ch_mp ~= state.expected_mp then
                return false
            end
            if caster:mp() ~= state.expected_mp then
                return false
            end

            local blocked = target:request(
                resp.position,
                protocol.move("RIGHT", target:oid(), target:position()),
                function(p)
                    return skill.positions_equal(p.abs, state.original_position)
                end)
            if blocked == false or blocked == nil then
                return false
            end
            if skill.positions_equal(target:position(), state.original_position) == false then
                return false
            end

            local dispel_slot = caster:learn_spell("활력")
            if dispel_slot == 0xFF then
                return false
            end

            state.expected_mp_dispel = caster:mp() - 30
            local dispel_result = caster:request(
                resp.update_internal,
                protocol.spell_cast("TARGET", dispel_slot, "", target:oid(), target:position()),
                function(p)
                    return p.ch_mp == state.expected_mp_dispel
                end)
            if dispel_result == false or dispel_result == nil then
                return false
            end
            if caster:mp() ~= state.expected_mp_dispel then
                return false
            end

            -- After dispel, a successful move does not emit position to self.
            -- Use move + brief wait; paralysis snapback resets position via on_position.
            local before_move = {target:position()[1], target:position()[2]}
            target:move("RIGHT", 1)
            target:sleep(200)
            if skill.positions_equal(target:position(), before_move) then
                return false
            end
            target:move("LEFT", 1)

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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "노도성황") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "투명") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "신의축복") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "백호령") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "진백호령") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "진백호령"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
    make_directional_attack_case("측면공격", {{-1, 0}, {1, 0}}),
    make_directional_attack_case("후면공격", {{0, -1}, {0, 1}}),
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "차폐") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "전혈") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "운기") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "심안투영") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "파력무참") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "신령지익") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "이중공격") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "삼중공격") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "사중공격") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "오중공격") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "분신") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "누리의힘") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
        clear_target_buffs_on_retry = false,
        pre = function(caster, _, state)
            if state.lure_slot == nil then
                caster:setup_bot_stats(100000, 100000)
                state.expected_hp = caster:base_hp()
                state.lure_slot = caster:learn_spell("유인")
                if state.lure_slot == 0xFF then
                    return false
                end
            end
            
            caster:set_current_hp_mp(caster:base_hp(), caster:base_mp())
            if caster:mp() < 600 then
                return false
            end
            return true
        end,
        condition = function(packet)
            if packet.text == nil then
                return nil
            end
            if packet.text == "실패" then
                return false
            end
            if skill.is_cast_ready(packet.text, "금강불체") then
                return true
            end
            return nil
        end,
        post = function(caster, _, state)
            local buff_result = caster:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "금강불체"
            end)
            if buff_result == false or buff_result == nil then
                return false
            end
            
            local spawn = caster:request(
            resp.update,
            protocol.chat(false, "/몬스터범위생성 초급유령 1"),
            function(packet)
                return packet.object_count == 4
            end)
            if spawn == false or spawn == nil then
                return false
            end
            
            for i = 1, spawn.object_count do
                local mob = spawn.objects_data[i]
                local lure_ok = caster:request(
                resp.action,
                protocol.spell_cast("TARGET", state.lure_slot, "", mob.oid, {mob.x, mob.y}),
                function(packet)
                    return packet.oid == caster:oid()
                end)
                if lure_ok == false or lure_ok == nil then
                    return false
                end
            end
            
            caster:sleep(3000)
            caster:chat("/몬스터범위제거 1")
            caster:sleep(500)
            
            if caster:hp() ~= state.expected_hp then
                return false
            end
            
            caster:remove_buffs()
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "무장") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "보호") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
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
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "대지의힘") then
                return true
            end
            return false
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                return false
            end
            state.buff_target:request(resp.spell_buff, protocol.self_info(), function(packet)
                return packet.name == "대지의힘"
            end)
            state.buff_target:remove_buffs()
            return true
        end,
    },
}

function M.run(ctx, bot_index, target_index)
    target_index = target_index or lib.formation.skill_pair_target(bot_index)
    if target_index == nil then
        log("debug", "BUFF/DEBUFF SPELL TEST requires a target bot")
        return false
    end
    local caster = ctx:bot(bot_index)
    local target = ctx:bot(target_index)
    log("debug", "BUFF/DEBUFF SPELL TEST STARTED")
    caster:setup_bot_stats(100000, 100000)
    target:setup_bot_stats(100000, 100000)
    if spell_runner.run_cases(CASES, caster, target, { ctx = ctx }) == false then
        return false
    end
    log("debug", "BUFF/DEBUFF SPELL TEST COMPLETED")
    return true
end
return M
