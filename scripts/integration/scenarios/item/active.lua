local resp     = require("integration.response")
local protocol = require("integration.protocol")

local M = {}

local ITEM_DONGDONGJU = "동동주"
local ITEM_UITAE      = "의태시약"
local EXPECTED_NAME_AFTER_USE = "동동주 [199]잔"

local TARGET_EQUIP = {
    "목도",
    "남자기모노",
    "연등",
}

local EXPECTED_DRESS = {
    weapon_dress = 1,
    armor_dress  = 55,
    shield_dress = 16,
}

local ACTIVE_MAP = "낙랑의방"

local function relocate_other_bots(ctx)
    local count = ctx:bot_count()
    if count <= 2 then
        return
    end

    -- Grid init places bots 12/13/14 at (6,6)/(7,6)/(8,6); stage the rest on lower rows.
    local start_x, start_y = 5, 12
    local width = 11
    for i = 2, count - 1 do
        local bot = ctx:bot(i)
        local offset = i - 2
        bot:map_move(ACTIVE_MAP, start_x + (offset % width), start_y + math.floor(offset / width))
    end
end

local function reset_bot(bot)
    bot:clear_inventory()
    bot:remove_buffs()
    bot:setup_bot_stats(10000, 10000)
    bot:gender("MALE")
end

local function clear_map_drops(bot)
    bot:clear_all_drop_items()
end

local function clear_map_monsters(bot)
    bot:chat("/몬스터제거")
end

local function restore_target_state(target, caster)
    if target:state() == "GHOST" then
        target:chat("/체력바꾸기 10000")
        local packet = target:request(
            resp.update_external_detailed,
            protocol.update_option("EXTENSION", true),
            function(p)
                return p.oid == target:oid() and p.state == "NORMAL"
            end)
        if packet == false or packet == nil then
            return false
        end
    elseif target:state() == "RIDING" then
        local packet = target:request(
            resp.update_external_detailed,
            protocol.update_option("EXTENSION", true),
            function(p)
                return p.oid == target:oid() and p.state == "NORMAL"
            end)
        if packet == false or packet == nil then
            return false
        end
        clear_map_monsters(caster)
    end
    return true
end

local function arrange_facing_pair(caster, target)
    caster:map_move(ACTIVE_MAP, 6, 6)
    target:map_move(ACTIVE_MAP, 7, 6)
    caster:direction("RIGHT")
    target:direction("RIGHT")
end

local function dress_matches(packet, expected)
    return packet.weapon_dress == expected.weapon_dress
        and packet.armor_dress == expected.armor_dress
        and packet.shield_dress == expected.shield_dress
end

local function dress_from_packet(packet)
    return {
        weapon_dress = packet.weapon_dress,
        armor_dress  = packet.armor_dress,
        shield_dress = packet.shield_dress,
    }
end

local function equip_target_loadout(target)
    for _, name in ipairs(TARGET_EQUIP) do
        target:create_item(name, 1)
        if target:equip(0) == false then
            log("fatal", string.format("uitae: equip failed for %s", name))
            return false
        end
    end
    return true
end

local function enter_ghost(target, caster)
    local option = require("integration.lib.option")
    if option.disable_pk_protect(caster) == false then
        log("fatal", "uitae: failed to disable PK_PROTECT for caster")
        return false
    end

    caster:setup_bot_stats(100000, 100000)
    target:set_current_hp_mp(50, target:mp())

    local hell_slot = caster:learn_spell("헬파이어")
    if hell_slot == 0xFF then
        log("fatal", "uitae: failed to learn 헬파이어")
        return false
    end

    local pos = target:position()
    local packet = caster:request(
        resp.update_external_detailed,
        protocol.spell_cast("TARGET", hell_slot, "", target:oid(), pos),
        function(pkt)
            return pkt.oid == target:oid() and pkt.state == "GHOST"
        end)
    if packet == false or packet == nil then
        log("fatal", "uitae: failed to put target into GHOST state")
        return false
    end

    clear_map_drops(caster)
    return dress_from_packet(packet)
end

local function enter_riding(target)
    -- Mount requires horse in front of target while target faces the horse.
    target:direction("RIGHT")
    target:spawn_monster_relative("말", 1, 0)

    local packet = target:request(
        resp.update_external_detailed,
        protocol.update_option("EXTENSION", true),
        function(pkt)
            return pkt.oid == target:oid() and pkt.state == "RIDING"
        end)
    if packet == false or packet == nil then
        log("fatal", "uitae: failed to put target into RIDING state")
        return false
    end
    return dress_from_packet(packet)
end

function M.run_dongdongju(ctx)
    local caster = ctx:bot(0)
    reset_bot(caster)

    caster:create_item(ITEM_DONGDONGJU, 1)
    local slot = caster:item_slot(ITEM_DONGDONGJU)
    if slot == 0xFF then
        log("fatal", "dongdongju: item not created")
        return false
    end

    local before_hp = caster:hp()
    local max_mp    = caster:base_mp()
    caster:set_current_hp_mp(before_hp, 0)
    if caster:mp() ~= 0 then
        log("fatal", "dongdongju: failed to set mp to 0")
        return false
    end

    local expected_hp = math.max(1, before_hp - 1)
    local expected_mp = math.min(30, max_mp)

    local packet = caster:request(
        resp.item_update,
        protocol.item_active(slot),
        function(p)
            return p.index == slot and p.name == EXPECTED_NAME_AFTER_USE
        end)
    if packet == false or packet == nil then
        log("fatal", "dongdongju: item_update timeout or name mismatch")
        return false
    end

    if caster:hp() ~= expected_hp then
        log("fatal", string.format(
            "dongdongju: hp expected=%d got=%d",
            expected_hp,
            caster:hp()))
        return false
    end
    if caster:mp() ~= expected_mp then
        log("fatal", string.format(
            "dongdongju: mp expected=%d got=%d",
            expected_mp,
            caster:mp()))
        return false
    end

    return true
end

local function run_uitae_variant(ctx, variant)
    local caster = ctx:bot(0)
    local target = ctx:bot(1)

    reset_bot(caster)
    reset_bot(target)
    if restore_target_state(target, caster) == false then
        log("fatal", string.format("uitae %s: failed to restore target state", variant))
        return false
    end
    clear_map_drops(caster)

    arrange_facing_pair(caster, target)
    if equip_target_loadout(target) == false then
        return false
    end

    local expected_dress = EXPECTED_DRESS
    local expected_state = nil
    if variant == "ghost" then
        local ghost_dress = enter_ghost(target, caster)
        if ghost_dress == false then
            return false
        end
        expected_dress = ghost_dress
        expected_state = "GHOST"
    elseif variant == "riding" then
        local riding_dress = enter_riding(target)
        if riding_dress == false then
            return false
        end
        expected_dress = riding_dress
    end

    caster:create_item(ITEM_UITAE, 1)
    local before_count = caster:item_count(ITEM_UITAE)
    if before_count ~= 1 then
        log("fatal", string.format("uitae %s: expected 1 potion before use", variant))
        return false
    end

    local slot = caster:item_slot(ITEM_UITAE)
    if slot == 0xFF then
        log("fatal", string.format("uitae %s: potion slot not found", variant))
        return false
    end

    local packet = caster:request(
        resp.update_external_detailed,
        protocol.item_active(slot),
        function(p)
            if p.oid ~= caster:oid() then
                return false
            end
            if dress_matches(p, expected_dress) == false then
                return false
            end
            if expected_state ~= nil and p.state ~= expected_state then
                return false
            end
            return true
        end)
    if packet == false or packet == nil then
        log("fatal", string.format("uitae %s: appearance mismatch timeout", variant))
        return false
    end

    caster:sleep(1000)

    if caster:item_count(ITEM_UITAE) ~= before_count - 1 then
        log("fatal", string.format("uitae %s: item not consumed", variant))
        return false
    end

    caster:remove_buffs()
    if restore_target_state(target, caster) == false then
        log("fatal", string.format("uitae %s: failed to clean up target state", variant))
        return false
    end
    clear_map_drops(caster)

    return true
end

function M.run_uitae(ctx)
    for _, variant in ipairs({ "normal", "ghost", "riding" }) do
        log("debug", string.format("uitae variant start: %s", variant))
        if run_uitae_variant(ctx, variant) == false then
            return false
        end
    end
    return true
end

function M.run(ctx)
    log("debug", "Item active test started")
    relocate_other_bots(ctx)

    if M.run_dongdongju(ctx) == false then
        return false
    end
    if M.run_uitae(ctx) == false then
        return false
    end
    log("debug", "Item active test completed")
    return true
end

return M
