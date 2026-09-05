-- Sinsoo castle ownership, entry matrix, occupancy, and siege PK helpers.
local M = {}

M.WEAPON_NAME_BY_DIVINE_BEAST = {
    [DIVINE_BEAST.VERMILION_BIRD] = '주작의검',
    [DIVINE_BEAST.BLACK_TORTOISE] = '현무의검',
    [DIVINE_BEAST.AZURE_DRAGON] = '청룡의검',
    [DIVINE_BEAST.WHITE_TIGER] = '백호의검',
}

M.CLOTHES_NAME_BY_DIVINE_BEAST = {
    [DIVINE_BEAST.VERMILION_BIRD] = '주작의옷',
    [DIVINE_BEAST.BLACK_TORTOISE] = '현무의옷',
    [DIVINE_BEAST.AZURE_DRAGON] = '청룡의옷',
    [DIVINE_BEAST.WHITE_TIGER] = '백호의옷',
}

M.ALL_CLOTHES_NAMES = { '주작의옷', '현무의옷', '청룡의옷', '백호의옷' }

M.TOTEM_BY_DIVINE_BEAST = {
    [DIVINE_BEAST.VERMILION_BIRD] = '주작',
    [DIVINE_BEAST.BLACK_TORTOISE] = '현무',
    [DIVINE_BEAST.AZURE_DRAGON] = '청룡',
    [DIVINE_BEAST.WHITE_TIGER] = '백호',
}

local DIVINE_BEAST_BY_TOTEM = {
    ['주작'] = DIVINE_BEAST.VERMILION_BIRD,
    ['현무'] = DIVINE_BEAST.BLACK_TORTOISE,
    ['청룡'] = DIVINE_BEAST.AZURE_DRAGON,
    ['백호'] = DIVINE_BEAST.WHITE_TIGER,
}

local ENTRANCE_FEE = 200000
local ENTRANCE_FEE_TO_OWNER = 150000

local function zone_of(name)
    if name:find('의눈', 1, true) then
        return 'eye'
    elseif name:find('지하수로', 1, true) then
        return 'sewer'
    elseif name:find('팔괘', 1, true) then
        return 'palgwae'
    elseif name:find('의심장', 1, true) then
        return 'heart'
    end
    return nil
end

function M.totem_of_map(map)
    if map == nil then
        return nil, nil
    end

    local name = map:model():name()
    local divine = map:model():siege_castle()
    for prefix, beast in pairs(DIVINE_BEAST_BY_TOTEM) do
        if name:sub(1, #prefix) == prefix then
            if divine == nil then
                divine = beast
            end
            return prefix, divine
        end
    end
    return nil, divine
end

function M.map_siege_castle(me)
    local map = me:map()
    if map == nil then
        return false
    end
    local option = map:model():option()
    return (option & MAP_OPTION.SIEGE_CASTLE) == MAP_OPTION.SIEGE_CASTLE
end

function M.map_pk_enabled(me)
    local map = me:map()
    if map == nil then
        return false
    end
    local model = map:model()
    local option = model:option()
    if (option & MAP_OPTION.ENABLE_PK) == MAP_OPTION.ENABLE_PK then
        return true
    end

    -- Siege maps: PK only while that divine beast's siege is active.
    -- Use siege_active() (sync) — never castle(), which can yield and break attack flow.
    if (option & MAP_OPTION.SIEGE_CASTLE) ~= MAP_OPTION.SIEGE_CASTLE then
        return false
    end

    local divine_beast = model:siege_castle()
    if divine_beast == nil then
        return false
    end

    return siege_active(divine_beast)
end

function M.is_friendly_clan(me, you)
    if me == nil or you == nil then
        return false
    end
    if not me:is(OBJECT_TYPE.CHARACTER) or not you:is(OBJECT_TYPE.CHARACTER) then
        return false
    end

    local my_clan = me:clan()
    local your_clan = you:clan()
    if my_clan == nil or your_clan == nil then
        return false
    end

    if my_clan:id() == your_clan:id() then
        return true
    end

    return my_clan:is_allied(your_clan)
end

function M.blocks_siege_friendly_fire(me, you)
    return M.map_siege_castle(me) and M.is_friendly_clan(me, you)
end

local function is_owner_or_ally(me, owner_clan_id)
    if owner_clan_id == nil then
        return false, false
    end

    local clan = me:clan()
    if clan == nil then
        return false, false
    end

    local is_owner = clan:id() == owner_clan_id
    local is_ally = clan:is_allied(owner_clan_id)
    return is_owner, is_ally
end

function M.can_occupy(me, map)
    if me == nil or map == nil then
        return false
    end

    local name = map:model():name()
    local zone = zone_of(name)
    if zone == nil then
        return true
    end

    local _, divine_beast = M.totem_of_map(map)
    local castle_obj = divine_beast ~= nil and castle(divine_beast) or nil
    if zone == 'heart' and castle_obj ~= nil and castle_obj:siege_active() then
        return true
    end

    local owner_clan_id = castle_obj and castle_obj:owner_clan() or nil
    local is_owner, is_ally = is_owner_or_ally(me, owner_clan_id)
    if not is_owner and not is_ally then
        return false
    end

    if (zone == 'sewer' or zone == 'palgwae') and is_owner and me:level() < 99 then
        return false
    end

    return true
end

function M.evict_to_entrance(me, totem)
    if me == nil or totem == nil then
        return
    end

    local dest = name2map(totem .. '의성입구')
    if dest == nil then
        return
    end

    me:message('성 내부에서 퇴장되었습니다.', MESSAGE_TYPE.NOTIFY)
    me:map(dest, math.random(44, 49), math.random(84, 90))
end

function M.evict(me)
    if me == nil then
        return
    end
    -- Login calls map() before characters.insert / clan attach. Evicting here
    -- would hop the character off the login thread and kick owners (no clan yet).
    if name2ch(me:name()) == nil then
        return
    end

    local map = me:map()
    if map == nil then
        return
    end
    if M.can_occupy(me, map) then
        return
    end

    local totem = select(1, M.totem_of_map(map))
    if totem == nil then
        return
    end
    M.evict_to_entrance(me, totem)
end

function M.evict_castle_maps(totem)
    local suffixes = { '의눈', '의심장', '지하수로', '팔괘의방', '팔괘방' }
    for _, suffix in ipairs(suffixes) do
        local map = name2map(totem .. suffix)
        if map ~= nil then
            local objs = map:objects(OBJECT_TYPE.CHARACTER)
            for _, ch in pairs(objs) do
                M.evict(ch)
            end
        end
    end
end

function M.enter_castle(me, npc_obj, totem_name_kr, divine_beast)
    local clan = me:clan()
    if clan == nil then
        me:dialog(npc_obj, '문파가 없으면 들어갈 수 없다네.')
        return true
    end
    if me:state() == STATE.GHOST then
        me:dialog(npc_obj, '유령은 참가할 수 없습니다.')
        return true
    end

    local castle_name = totem_name_kr .. '의성'
    local castle_obj = castle(divine_beast)
    local siege_active = castle_obj and castle_obj:siege_active()

    if siege_active then
        local map_entrance = name2map(totem_name_kr .. '의성입구')
        if map_entrance == nil then
            me:dialog(npc_obj, '입장할 수 있는 맵이 없습니다.')
            return true
        end
        me:map(map_entrance, math.random(14, 97), math.random(206, 240))
    else
        local owner_clan_id = castle_obj and castle_obj:owner_clan()
        if owner_clan_id == nil then
            me:dialog(npc_obj, '성의 주인이 없는것 같군.')
            return true
        end

        local owner_clan = id2clan(owner_clan_id)
        local owner_name = owner_clan and owner_clan:name() or ''
        local map_eye = name2map(totem_name_kr .. '의눈')
        if map_eye == nil then
            me:dialog(npc_obj, '입장할 수 있는 맵이 없습니다.')
            return true
        end

        if clan:id() == owner_clan_id then
            me:map(map_eye, math.random(12, 16), math.random(22, 31))
        elseif clan:is_allied(owner_clan_id) then
            local button = me:dialog(npc_obj, string.format('자네는 이 성을 차지하고 있는 [%s] 문파의 동맹 문파원 자격으로 %s에 입장할 수 있다네. 자 그럼 들여보내 주겠네.', owner_name, castle_name), { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return true
            end
            me:map(map_eye, math.random(12, 16), math.random(22, 25))
        else
            local button = me:dialog(npc_obj, '성을 점령중인 문파나 그 동맹문파만 입장이 가능합니다.\n\n만약 입장료를 내고서라도 입장하시겠다면, 입장을 시켜드릴 수 있습니다.', { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return true
            end

            local selected = me:pursuit(npc_obj, string.format('[%s] 입장료 20만전을 [%s]문파에게 지불합니다. 입장하시겠습니까?', castle_name, owner_name), { '입장합니다.', '아니오.' })
            if selected ~= 1 then
                return true
            end

            if me:money() < ENTRANCE_FEE then
                me:dialog(npc_obj, '보유하고 계신 금전이 없어, 입장료를 지불하실 수 없습니다. 입장료는 금전 20만전이 필요합니다.')
                return true
            end

            local map_inner = name2map(castle_name)
            if map_inner == nil then
                me:dialog(npc_obj, '입장할 수 있는 맵이 없습니다.')
                return true
            end

            me:money(me:money() - ENTRANCE_FEE)
            if owner_clan ~= nil then
                local _, err = owner_clan:money(ENTRANCE_FEE_TO_OWNER)
                if err ~= nil then
                    me:money(me:money() + ENTRANCE_FEE)
                    me:dialog(npc_obj, '문파자금 적립에 실패하여 입장할 수 없습니다.')
                    return true
                end
            end

            me:map(map_inner, math.random(74, 84), math.random(117, 130))
        end
    end
    return true
end

local function remove_all_by_name(ch, item_name)
    local removed = false

    for parts, item in pairs(ch:equipments()) do
        if item:model():name() == item_name then
            if ch:equipment_off(parts) == nil then
                return removed
            end
        end
    end

    for _, item in pairs(ch:items(item_name)) do
        if ch:rmitem(item, item:count(), ITEM_DELETE_TYPE.REMOVED) then
            removed = true
        end
    end

    return removed
end

function M.has_item_named(ch, item_name)
    if #ch:items(item_name) > 0 then
        return true
    end
    for _, item in pairs(ch:equipments()) do
        if item:model():name() == item_name then
            return true
        end
    end
    return false
end

function M.strip(ch, divine_beast)
    local castle_obj = castle(divine_beast)
    local owner_clan_id = castle_obj and castle_obj:owner_clan()
    local clan = ch:clan()
    local clan_id = clan and clan:id() or nil
    local owns_castle = owner_clan_id ~= nil and clan_id == owner_clan_id
    if owns_castle then
        return
    end

    local weapon_name = M.WEAPON_NAME_BY_DIVINE_BEAST[divine_beast]
    if weapon_name ~= nil and remove_all_by_name(ch, weapon_name) then
        ch:message(string.format('성 소유권이 변경되어 %s 회수되었습니다.', name_with(weapon_name, '이', '가')))
    end

    local clothes_name = M.CLOTHES_NAME_BY_DIVINE_BEAST[divine_beast]
    if clothes_name ~= nil and remove_all_by_name(ch, clothes_name) then
        ch:message(string.format('성 소유권이 변경되어 %s 회수되었습니다.', name_with(clothes_name, '이', '가')))
    end
end

function M.strip_all(ch)
    for divine_beast, _ in pairs(M.WEAPON_NAME_BY_DIVINE_BEAST) do
        M.strip(ch, divine_beast)
    end
end

return M
