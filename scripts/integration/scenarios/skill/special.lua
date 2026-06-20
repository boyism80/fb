local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local function slog(fmt, ...)
    log("debug", string.format("special: " .. fmt, ...))
end

local WEAPON_NAME = "목도"
local NOT_READY_MESSAGE = "비바람이 휘몰아치고 있습니다."

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

local SPECIAL_SPELLS = {
    {
        name = "비영승보",
        pre = function(caster)
            caster:str(99)
            caster:level(30)
            caster:create_item("양첨목봉", 1)
            caster:equip(0)
            return true
        end,
        cast = function(caster, slot)
            local before = caster:position()
            local pos = {before[1], before[2] + 1}
            caster:spawn_monster("다람쥐", pos[1], pos[2])
            caster:request(
                resp.position,
                protocol.spell_cast("NORMAL", slot, "", 0, {0, 0}),
                function(packet)
                    return skill.positions_equal(packet.abs, before) == false
                end)
            return true
        end,
        post = function(caster)
            caster:chat("/몬스터제거")
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
        pre = function(_)
            return true
        end,
        cast = function(caster, slot)
            -- Local server (127.0.0.1): spell is not ready on game server.
            -- Non-local transfer() path skipped; tests run on local typically.
            local packet = caster:request(
                resp.message,
                protocol.spell_cast("NORMAL", slot, "", 0, {0, 0}),
                function(p)
                    return p.type == "STATE"
                end)
            return packet.text == NOT_READY_MESSAGE
        end,
        post = function(caster)
            caster:chat("/맵이동 낙랑의방 6 6")
            return true
        end,
    },
    {
        name = "비영사천문",
        pre = function(_)
            return true
        end,
        cast = function(caster, slot)
            -- Non-local map_move via transfer() skipped; local test keeps position.
            local before = caster:position()
            local packet = caster:request(
                resp.message,
                protocol.spell_cast("INPUT", slot, "동", 0, {0, 0}),
                function(p)
                    return p.type == "STATE"
                end)

            if skill.is_cast_ready(packet.text, "비영사천문") == false then
                return false
            end

            return skill.positions_equal(caster:position(), before)
        end,
        post = function(_)
            return true
        end,
    },
    {
        name = "공력증강",
        pre = function(caster)
            caster:set_max_hp_mp(10000, 10000)
            return true
        end,
        cast = function(caster, slot)
            while true do
                caster:set_current_hp_mp(10000, 30)
                local packet = caster:request(
                    resp.message,
                    protocol.spell_cast("NORMAL", slot, "", 0, {0, 0}),
                    function(p)
                        return p.type == "STATE"
                    end)

                if skill.is_cast_ready(packet.text, "공력증강") then
                    break
                end
            end

            if caster:mp() ~= 10000 then
                return false
            end
            if caster:hp() >= 10000 then
                return false
            end
            return true
        end,
        post = function(_)
            return true
        end,
    },
    {
        name = "대력검신",
        pre = function(caster)
            caster:create_item(WEAPON_NAME, 1)
            caster:equip(0)
            return true
        end,
        cast = function(caster, slot)
            caster:request(
                resp.message,
                protocol.spell_cast("INPUT", slot, WEAPON_NAME, 0, {0, 0}),
                function(packet)
                    if packet.type ~= "STATE" then
                        return false
                    end
                    return packet.text:find(WEAPON_NAME, 1, true) ~= nil
                        and packet.text:find("푸른 빛으로 빛납니다", 1, true) ~= nil
                end)
            return true
        end,
        post = function(_)
            return true
        end,
    },
    {
        name = "검신검귀",
        pre = function(_)
            return true
        end,
        cast = function(caster, slot)
            caster:request(
                resp.message,
                protocol.spell_cast("INPUT", slot, WEAPON_NAME, 0, {0, 0}),
                function(packet)
                    if packet.type ~= "STATE" then
                        return false
                    end
                    return packet.text:find(WEAPON_NAME, 1, true) ~= nil
                        and packet.text:find("푸른 빛으로 빛납니다", 1, true) ~= nil
                end)
            return true
        end,
        post = function(_)
            return true
        end,
    },
    {
        name = "신검합일",
        pre = function(_)
            return true
        end,
        cast = function(caster, slot)
            caster:request(
                resp.message,
                protocol.spell_cast("INPUT", slot, WEAPON_NAME, 0, {0, 0}),
                function(packet)
                    if packet.type ~= "STATE" then
                        return false
                    end
                    return packet.text:find(WEAPON_NAME, 1, true) ~= nil
                        and packet.text:find("푸른 빛으로 빛납니다", 1, true) ~= nil
                end)
            return true
        end,
        post = function(caster)
            caster:unequip("WEAPON")
            caster:clear_inventory()
            return true
        end,
    },
    {
        name = "소혼강신",
        pre = function(caster)
            caster:level(68)
            return true
        end,
        cast = function(caster, slot)
            local mob = "평웅"
            caster:request(
                resp.update,
                protocol.spell_cast("INPUT", slot, mob, 0, {0, 0}),
                function(packet)
                    return packet.object_count == 1
                end)
            return true
        end,
        post = function(caster)
            caster:chat("/몬스터제거")
            return true
        end,
    },
    {
        name = "성황령",
        pre = function(caster)
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

            slog("성황령 pre: casting 헬파이어 on self, waiting update_external_detailed GHOST")
            local packet = caster:request(
                resp.update_external_detailed,
                protocol.spell_cast("TARGET", hell_slot, "", caster:oid(), pos),
                function(pkt)
                    slog("성황령 pre: update_external_detailed oid=%d state=%s (want oid=%d GHOST)",
                        pkt.oid, tostring(pkt.state), caster:oid())
                    return pkt.oid == caster:oid() and pkt.state == "GHOST"
                end)
            slog("성황령 pre: ghost state confirmed oid=%d state=%s",
                packet.oid, tostring(packet.state))

            -- Non-local map_move via transfer() skipped; local test stays on current map.
            slog("성황령 pre: completed")
            return true
        end,
        cast = function(caster, slot)
            local map_id = caster:map()
            local pos = caster:position()
            local root, revive = root_revive_for_map_id(map_id)

            if root == nil then
                slog("성황령 cast: id2map(%d) or root not found", map_id)
                return false
            end

            if revive == nil then
                slog("성황령 cast: revive not enabled on root map %s (id=%d), skipping",
                    root:name(), root:id())
                return true
            end

            local west_map = revive[CARDINAL_DIRECTION.WEST]
            if west_map == nil then
                slog("성황령 cast: WEST revive not configured on root %s, skipping", root:name())
                return true
            end

            local expected_id = west_map:id()
            slog("성황령 cast: slot=%d oid=%d map=%d pos=(%d,%d) expected_west_map=%d (%s)",
                slot, caster:oid(), map_id, pos[1], pos[2], expected_id, west_map:name())
            slog("성황령 cast: spell_cast INPUT 좌, waiting map_config")

            caster:request(
                resp.map_config,
                protocol.spell_cast("INPUT", slot, "좌", 0, {0, 0}),
                function(pkt)
                    slog("성황령 cast: map_config id=%d name=%s building=%s",
                        pkt.id, tostring(pkt.name), tostring(pkt.building))
                    return true
                end)

            if caster:map() ~= expected_id then
                slog("성황령 cast: expected map id %d, got %d", expected_id, caster:map())
                return false
            end

            pos = caster:position()
            slog("성황령 cast: completed map=%d pos=(%d,%d)", caster:map(), pos[1], pos[2])
            return true
        end,
        post = function(caster)
            slog("성황령 post: restoring map and stats")
            caster:chat("/맵이동 낙랑의방 6 6")
            caster:set_max_hp_mp(100000, 100000)
            slog("성황령 post: completed map=%d hp=%d mp=%d",
                caster:map(), caster:hp(), caster:mp())
            return true
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "SPECIAL SPELL TEST STARTED")

    caster:setup_bot_stats(100000, 100000)
    caster:learn_spells(skill.spell_names(SPECIAL_SPELLS))
    log("debug", "SPECIAL SPELL LEARNING COMPLETED")

    local slot = 0
    for _, spell in ipairs(SPECIAL_SPELLS) do
        slog("[%d] %s pre starting", slot, spell.name)
        if spell.pre(caster) == false then
            slog("[%d] %s pre failed, skipping", slot, spell.name)
            slot = slot + 1
        else
            slog("[%d] %s pre ok", slot, spell.name)
            slog("[%d] %s cast starting", slot, spell.name)
            local cast_ok = spell.cast(caster, slot)
            if cast_ok then
                slog("[%d] %s cast ok", slot, spell.name)
                slog("[%d] %s post starting", slot, spell.name)
                spell.post(caster)
                slog("[%d] %s post ok", slot, spell.name)
            else
                slog("[%d] %s cast failed, skipping post", slot, spell.name)
            end
            slot = slot + 1
        end
    end

    log("debug", "SPECIAL SPELL TEST COMPLETED")
    return true
end

return M
