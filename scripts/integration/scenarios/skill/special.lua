local spell_runner = require("integration.lib.spell_runner")
local bot_diag     = require("integration.lib.bot_diag")

local M = {}

local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local function slog(fmt, ...)
    log("debug", string.format("special: " .. fmt, ...))
end

local WEAPON_NAME = "목도"
local NOT_READY_MESSAGE = "비바람이 휘몰아치고 있습니다."
local NAKRANG_ROOM = "낙랑의방"
local NEARBY_MOB_CLEAR_RANGE = 3
local g_suite_slot = 0

local function restore_position(caster, map_name, position)
    slog("restore_position: enter target=%s pos=(%s,%s) suite_slot=%s",
        tostring(map_name),
        tostring(position and position[1]),
        tostring(position and position[2]),
        tostring(g_suite_slot))
    bot_diag.dump(caster, "restore_position:before")

    if map_name == nil or position == nil then
        slog("restore_position: missing map_name or position")
        bot_diag.dump(caster, "restore_position:missing_target")
        return false
    end

    local map_id = caster:map()
    local map_model = id2map(map_id)
    if map_model ~= nil then
        local current = caster:position()
        local current_name = map_model:name()
        slog("restore_position: current map=%s (id=%d) pos=(%d,%d) target map=%s pos=(%d,%d)",
            current_name, map_id, current[1], current[2],
            map_name, position[1], position[2])
    else
        slog("restore_position: current map id=%s unknown (0xFFFF=uninitialized?); forcing move to target",
            tostring(map_id))
    end

    local dest_model = id2map(caster:map())
    local dest_pos = caster:position()
    if dest_model ~= nil
        and dest_model:name() == map_name
        and dest_pos[1] == position[1]
        and dest_pos[2] == position[2] then
        slog("restore_position: already at dest")
        return true
    end

    -- Motel maps from 귀환 are often on hosts that cannot create seat instances.
    -- Step 1: transfer to source 낙랑의방 (no slot) so the host can serve the map.
    -- Step 2: local map_move into suite_slot instance.
    local cmd_src = string.format("/맵이동 %s %d %d", map_name, position[1], position[2])
    slog("restore_position: branch=transfer_src cmd=%s", cmd_src)
    local ok, err = pcall(function()
        caster:transfer(protocol.chat(false, cmd_src))
    end)
    dest_model = id2map(caster:map())
    if ok == false and (dest_model == nil or dest_model:name() ~= map_name) then
        slog("restore_position: transfer_src FAILED err=%s", tostring(err))
        bot_diag.dump(caster, "restore_position:transfer_src_failed")
        return false
    end
    dest_model = id2map(caster:map())
    if dest_model == nil or dest_model:name() ~= map_name then
        slog("restore_position: not on target map after transfer_src")
        bot_diag.dump(caster, "restore_position:transfer_src_map_mismatch")
        return false
    end

    slog("restore_position: branch=map_move_slot suite_slot=%s", tostring(g_suite_slot))
    local move_ok, move_err = pcall(function()
        caster:map_move(map_name, position[1], position[2], g_suite_slot)
    end)
    dest_model = id2map(caster:map())
    dest_pos = caster:position()
    if dest_model == nil
        or dest_model:name() ~= map_name
        or dest_pos[1] ~= position[1]
        or dest_pos[2] ~= position[2] then
        slog("restore_position: map_move_slot failed err=%s", tostring(move_err))
        bot_diag.dump(caster, "restore_position:slot_failed")
        return false
    end

    slog("restore_position: done")
    bot_diag.dump(caster, "restore_position:after")
    return true
end

local function restore_nakrang_room(caster)
    local map_model = id2map(caster:map())
    if map_model == nil then
        slog("restore_nakrang_room: id2map(%s) not found", tostring(caster:map()))
        bot_diag.dump(caster, "restore_nakrang_room:unknown_map")
        return false
    end

    if map_model:name() ~= NAKRANG_ROOM then
        local ok = restore_position(caster, NAKRANG_ROOM, {6, 6})
        if ok == false then
            return false
        end
    end
    return true
end

local function root_revive_for_map_id(map_id)
    local map_model = id2map(map_id)
    if map_model == nil then
        return nil, nil
    end
    
    local root = map_model:root()
    if root == nil then
        return nil, nil
    end
    
    return root, root:revive()
end

local function weapon_enchant_condition(packet)
    if packet.type ~= "STATE" then
        return nil
    end
    if packet.text:find(WEAPON_NAME, 1, true) ~= nil
    and packet.text:find("푸른 빛으로 빛납니다", 1, true) ~= nil then
        return true
    end
    return nil
end

local SPECIAL_SPELLS = {
    {
        name = "비영승보",
        response = resp.position,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:str(99)
            caster:level(30)
            caster:create_item("양첨목봉", 1)
            caster:equip(0)
            state.before = caster:position()
            local pos = {state.before[1], state.before[2] + 1}
            caster:spawn_monster("다람쥐", pos[1], pos[2])
        end,
        condition = function(packet, _, _, state)
            if skill.positions_equal(packet.abs, state.before) then
                return nil
            end
            return true
        end,
        post = function(caster)
            caster:chat(string.format("/몬스터범위제거 %d", NEARBY_MOB_CLEAR_RANGE))
            caster:clear_all_drop_items()
            caster:unequip("WEAPON")
            caster:clear_inventory()
            caster:move("TOP", 2)
            caster:direction("BOTTOM")
            return true
        end,
    },
    {
        name = "귀환",
        cast = function(caster, _, slot, state)
            if localhost() then
                local packet = caster:request(
                resp.message,
                protocol.spell_cast("NORMAL", slot, "", 0, {0, 0}),
                function(p)
                    log("debug", "귀환: p.text=" .. p.text)
                    log("debug", "귀환: p.type=" .. p.type)
                    return p.type == "STATE" and p.text == NOT_READY_MESSAGE
                end)
                state.packet = packet
                return true
            end

            bot_diag.dump(caster, "귀환:before_cast")
            local map_model = id2map(caster:map())
            if map_model == nil then
                slog("귀환: id2map(%s) not found before cast", tostring(caster:map()))
                bot_diag.dump(caster, "귀환:unknown_map_before_cast")
                return false
            end
            state.return_map_name = map_model:name()
            state.return_pos = caster:position()
            slog("귀환: casting; will restore to map=%s pos=(%d,%d) suite_slot=%s",
                state.return_map_name,
                state.return_pos[1],
                state.return_pos[2],
                tostring(g_suite_slot))

            local ok, err = pcall(function()
                caster:transfer(protocol.spell_cast("NORMAL", slot, "", 0, {0, 0}))
            end)
            if ok == false then
                slog("귀환: cast transfer FAILED err=%s", tostring(err))
                bot_diag.dump(caster, "귀환:cast_transfer_failed")
                return false
            end
            bot_diag.dump(caster, "귀환:after_cast_transfer")
            return true
        end,
        post = function(caster, _, state)
            if localhost() then
                if state.packet == nil or state.packet.text ~= NOT_READY_MESSAGE then
                    return false
                end
                return true
            end

            bot_diag.dump(caster, "귀환:post_before_restore")
            local ok = restore_position(caster, state.return_map_name, state.return_pos)
            if ok == false then
                bot_diag.dump(caster, "귀환:post_restore_failed")
            end
            return ok
        end,
    },
    {
        name = "비영사천문",
        cast = function(caster, _, slot, state)
            state.before = caster:position()
            
            if not localhost() then
                caster:transfer(protocol.chat(false, "/맵이동 국내성"))
            end
            
            local packet = caster:request(
            resp.message,
            protocol.spell_cast("INPUT", slot, "동", 0, {0, 0}),
            function(p)
                return p.type == "STATE"
            end)
            state.packet = packet
            
            if skill.is_cast_ready(packet.text, "비영사천문") == false then
                slog("비영사천문: cast not ready: p.text=" .. tostring(packet and packet.text))
                bot_diag.dump(caster, "비영사천문:cast_not_ready")
                return false
            end
            
            if localhost() then
                return skill.positions_equal(caster:position(), state.before)
            end
            
            if skill.positions_equal(caster:position(), state.before) then
                slog("비영사천문: position unchanged after cast")
                bot_diag.dump(caster, "비영사천문:position_unchanged")
                return false
            end
            
            return restore_nakrang_room(caster)
        end,
    },
    {
        name = "공력증강",
        response = resp.message,
        cast_type = "NORMAL",
        pre = function(caster)
            caster:set_max_hp_mp(10000, 10000)
            caster:set_current_hp_mp(10000, 30)
        end,
        condition = function(packet)
            if packet.text == nil then
                return nil
            end
            if skill.is_cast_ready(packet.text, "공력증강") then
                return true
            elseif packet.text == "정신집중에 실패하였습니다." then
                return false
            else
                return nil
            end
        end,
        post = function(caster)
            if caster:mp() ~= caster:base_mp() then
                return false
            end
            if caster:hp() >= 10000 then
                return false
            end
            return true
        end,
    },
    {
        name = "대력검신",
        response = resp.message,
        cast_type = "INPUT",
        message = WEAPON_NAME,
        pre = function(caster)
            caster:create_item(WEAPON_NAME, 1)
            caster:equip(0)
        end,
        condition = weapon_enchant_condition,
    },
    {
        name = "검신검귀",
        response = resp.message,
        cast_type = "INPUT",
        message = WEAPON_NAME,
        condition = weapon_enchant_condition,
    },
    {
        name = "신검합일",
        response = resp.message,
        cast_type = "INPUT",
        message = WEAPON_NAME,
        condition = weapon_enchant_condition,
        post = function(caster)
            caster:unequip("WEAPON")
            caster:clear_inventory()
            return true
        end,
    },
    {
        name = "소혼강신",
        response = resp.update,
        cast_type = "INPUT",
        message = "평웅",
        pre = function(caster)
            caster:level(68)
        end,
        condition = function(packet)
            if packet.object_count == 1 then
                return true
            end
            return nil
        end,
        post = function(caster)
            caster:chat(string.format("/몬스터범위제거 %d", NEARBY_MOB_CLEAR_RANGE))
            return true
        end,
    },
    {
        name = "성황령",
        response = resp.map_config,
        cast_type = "INPUT",
        message = "좌",
        should_skip = function(caster, _, state)
            local map_id = caster:map()
            local root, revive = root_revive_for_map_id(map_id)
            if root == nil then
                slog("성황령 should_skip: id2map(%d) or root not found", map_id)
                return false
            end
            if revive == nil then
                slog("성황령 should_skip: revive not enabled on root map %s (id=%d), skipping",
                root:name(), root:id())
                return true
            end
            
            local west_map = revive[CARDINAL_DIRECTION.WEST]
            if west_map == nil then
                slog("성황령 should_skip: WEST revive not configured on root %s, skipping", root:name())
                return true
            end
            
            state.expected_id = west_map:id()
            slog("성황령 should_skip: expected_west_map=%d (%s)", state.expected_id, west_map:name())
            return false
        end,
        pre = function(caster, _, state)
            slog("성황령 pre: learning 헬파이어")
            local hell_slot = caster:learn_spell("헬파이어")
            if hell_slot == 0xFF then
                slog("성황령 pre: failed to learn 헬파이어")
                return false
            end
            slog("성황령 pre: hell_slot=%d", hell_slot)
            
            caster:set_current_hp_mp(50, 100000)
            local pos = caster:position()
            slog("성황령 pre: oid=%d map=%d hp=%d mp=%d pos=(%d,%d)",
            caster:oid(), caster:map(), caster:hp(), caster:mp(), pos[1], pos[2])
            
            slog("성황령 pre: casting 헬파이어 on self, waiting show GHOST")
            local packet = caster:request(
            resp.show,
            protocol.spell_cast("TARGET", hell_slot, "", caster:oid(), pos),
            function(pkt)
                slog("성황령 pre: show oid=%d state=%s (want oid=%d GHOST)",
                pkt.oid, tostring(pkt.state), caster:oid())
                return pkt.oid == caster:oid() and pkt.state == "GHOST"
            end)
            slog("성황령 pre: ghost state confirmed oid=%d state=%s",
            packet.oid, tostring(packet.state))
            return true
        end,
        condition = function(packet)
            slog("성황령 cast: map_config id=%d name=%s building=%s",
            packet.id, tostring(packet.name), tostring(packet.building))
            return true
        end,
        post = function(caster, _, state)
            if caster:map() ~= state.expected_id then
                slog("성황령 cast: expected map id %d, got %d", state.expected_id, caster:map())
                return false
            end
            
            local pos = caster:position()
            slog("성황령 cast: completed map=%d pos=(%d,%d)", caster:map(), pos[1], pos[2])
            
            slog("성황령 post: restoring map and stats")
            caster:chat(string.format("/맵이동 낙랑의방 6 6 %d", g_suite_slot))
            caster:set_max_hp_mp(100000, 100000)
            slog("성황령 post: completed map=%d hp=%d mp=%d",
            caster:map(), caster:hp(), caster:mp())
            return true
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    g_suite_slot = ctx:suite_slot()
    log("debug", "SPECIAL SPELL TEST STARTED")
    local option = require("integration.lib.option")
    if option.disable_pk_protect(caster) == false then
        log("fatal", "SPECIAL SPELL TEST: failed to disable PK_PROTECT for caster")
        return false
    end
    caster:setup_bot_stats(100000, 100000)
    if spell_runner.run_cases(SPECIAL_SPELLS, caster, caster) == false then
        return false
    end
    log("debug", "SPECIAL SPELL TEST COMPLETED")
    return true
end
return M
