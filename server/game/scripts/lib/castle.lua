-- Sinsoo weapon (신수무기) ownership enforcement for castle-owning clans.
local M = {}

M.WEAPON_NAME_BY_DIVINE_BEAST = {
    [DIVINE_BEAST.VERMILION_BIRD] = '주작의검',
    [DIVINE_BEAST.BLACK_TORTOISE] = '현무의검',
    [DIVINE_BEAST.AZURE_DRAGON] = '청룡의검',
    [DIVINE_BEAST.WHITE_TIGER] = '백호의검',
}

local ENTRANCE_FEE = 200000
local ENTRANCE_FEE_TO_OWNER = 150000

local function pay_entrance_fee_to_owner(owner_clan)
    if owner_clan == nil then
        return
    end

    for _, member in pairs(owner_clan:members()) do
        if member:role() == CLAN_ROLE.MASTER then
            local master = name2ch(member:name())
            if master ~= nil then
                master:money(master:money() + ENTRANCE_FEE_TO_OWNER)
            end
            return
        end
    end
end

function M.enter_castle(me, npc_obj, totem_name_kr, divine_beast)
    local clan = me:clan()
    if clan == nil then
        me:dialog(npc_obj, '가입된 문파가 없습니다.')
        return true
    end
    if me:state() == STATE.GHOST then
        me:dialog(npc_obj, '유령은 참가할 수 없습니다.')
        return true
    end

    local castle_name = totem_name_kr .. '성'
    local map_entrance = name2map(totem_name_kr .. '성입구')
    if map_entrance == nil then
        me:dialog(npc_obj, '입장할 수 있는 맵이 없습니다.')
        return true
    end

    local castle = castle(divine_beast)
    local siege_active = castle and castle:siege_active()

    if siege_active then
        me:map(map_entrance, math.random(49, 57), math.random(145, 148))
        return true
    end

    local owner_clan_id = castle and castle:owner_clan()
    if owner_clan_id == nil then
        me:dialog(npc_obj, string.format('%s 주인이 없는것 같군.', castle_name))
        return true
    end

    local map_inner = name2map(totem_name_kr .. '의성')
    if map_inner == nil then
        me:dialog(npc_obj, '입장할 수 있는 맵이 없습니다.')
        return true
    end

    if clan:id() == owner_clan_id or clan:is_allied(owner_clan_id) then
        me:map(map_inner, math.random(11, 17), math.random(4, 11))
        return true
    end

    local owner_clan = id2clan(owner_clan_id)
    local owner_name = owner_clan and owner_clan:name() or ''

    local OPT_YES = '입장합니다.'
    local OPT_NO = '아니오.'
    local selected = me:pursuit(npc_obj,
        string.format('[%s] 입장료 %d전을 [%s]문파에게 지불합니다. 입장하시겠습니까?', castle_name, ENTRANCE_FEE, owner_name),
        { OPT_YES, OPT_NO })
    if selected ~= OPT_YES then
        return true
    end

    if me:money() < ENTRANCE_FEE then
        me:dialog(npc_obj, string.format('보유하고 계신 금전이 없어, 입장료를 지불하실 수 없습니다. 입장료는 금전 %d전이 필요합니다.', ENTRANCE_FEE))
        return true
    end

    me:money(me:money() - ENTRANCE_FEE)
    pay_entrance_fee_to_owner(owner_clan)

    me:map(map_inner, math.random(11, 17), math.random(4, 11))
    return true
end

local function remove_all_by_name(ch, weapon_name)
    local removed = false

    for parts, item in pairs(ch:equipments()) do
        if item:model():name() == weapon_name then
            if ch:equipment_off(parts) == nil then
                return removed
            end
        end
    end

    for _, item in pairs(ch:items(weapon_name)) do
        if ch:rmitem(item, item:count(), ITEM_DELETE_TYPE.REMOVED) then
            removed = true
        end
    end

    return removed
end

function M.enforce_ownership(ch, divine_beast)
    local weapon_name = M.WEAPON_NAME_BY_DIVINE_BEAST[divine_beast]
    if weapon_name == nil then
        return
    end

    local castle = castle(divine_beast)
    local owner_clan_id = castle and castle:owner_clan()

    local clan = ch:clan()
    local clan_id = clan and clan:id() or nil

    local owns_castle = owner_clan_id ~= nil and clan_id == owner_clan_id
    if owns_castle then
        return
    end

    if remove_all_by_name(ch, weapon_name) then
        ch:message(string.format('성 소유권이 변경되어 %s이(가) 회수되었습니다.', weapon_name))
    end
end

function M.enforce_all(ch)
    for divine_beast, _ in pairs(M.WEAPON_NAME_BY_DIVINE_BEAST) do
        M.enforce_ownership(ch, divine_beast)
    end
end

return M
