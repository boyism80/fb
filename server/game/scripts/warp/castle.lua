-- Castle (신수성) dest=script warps. Trash @script parity + intended siege blocks.
local castle_lib = require('lib.castle')

local HEAD_TO_EYE = {
    ['청룡의머리'] = { map = '청룡의눈', x1 = 4, x2 = 8, y = 2 },
    ['주작의머리'] = { map = '주작의눈', x1 = 4, x2 = 8, y = 2 },
    ['현무의머리'] = { map = '현무의눈', x1 = 4, x2 = 8, y = 2 },
    ['백호의머리'] = { map = '백호의눈', x1 = 4, x2 = 8, y = 2 },
}

local EYE_TO_HEAD = {
    ['청룡의눈'] = { map = '청룡의머리', x1 = 34, x2 = 37, y = 3 },
    ['주작의눈'] = { map = '주작의머리', x1 = 34, x2 = 37, y = 3 },
    ['현무의눈'] = { map = '현무의머리', x1 = 34, x2 = 37, y = 3 },
    ['백호의눈'] = { map = '백호의머리', x1 = 34, x2 = 37, y = 3 },
}

local HEART_DENEY = {
    ['주작의심장'] = { map = '주작의머리', x1 = 3, x2 = 5, y = 4 },
    ['현무의심장'] = { map = '현무의머리', x1 = 3, x2 = 5, y = 4 },
    ['청룡의심장'] = { map = '청룡의머리', x1 = 3, x2 = 5, y = 4 },
    ['백호의심장'] = { map = '백호의머리', x1 = 3, x2 = 5, y = 4 },
}

local HEART_OUT = {
    ['주작의심장'] = { map = '주작의성', x1 = 72, x2 = 75, y = 50 },
    ['현무의심장'] = { map = '현무의성', x1 = 72, x2 = 75, y = 50 },
    ['청룡의심장'] = { map = '청룡의성', x1 = 72, x2 = 75, y = 50 },
    ['백호의심장'] = { map = '백호의성', x1 = 72, x2 = 75, y = 50 },
}

local SEWER_IN = {
    ['주작의심장'] = { map = '주작지하수로', x1 = 1, x2 = 3, y = 12 },
    ['현무의심장'] = { map = '현무지하수로', x1 = 1, x2 = 3, y = 12 },
    ['청룡의심장'] = { map = '청룡지하수로', x1 = 1, x2 = 3, y = 12 },
    ['백호의심장'] = { map = '백호지하수로', x1 = 1, x2 = 3, y = 12 },
}

local SEWER_OUT = {
    ['주작지하수로'] = { map = '주작의심장', x1 = 19, x2 = 21, y = 6 },
    ['현무지하수로'] = { map = '현무의심장', x1 = 19, x2 = 21, y = 6 },
    ['청룡지하수로'] = { map = '청룡의심장', x1 = 19, x2 = 21, y = 6 },
    ['백호지하수로'] = { map = '백호의심장', x1 = 19, x2 = 21, y = 6 },
}

local PALGWAE = {
    ['주작의성'] = { map = '주작팔괘의방', x1 = 16, x2 = 17, y = 29 },
    ['현무의성'] = { map = '현무팔괘의방', x1 = 16, x2 = 17, y = 29 },
    ['청룡의성'] = { map = '청룡팔괘의방', x1 = 16, x2 = 17, y = 29 },
    ['백호의성'] = { map = '백호팔괘의방', x1 = 16, x2 = 17, y = 29 },
}

local FORGE = {
    ['주작의성'] = { map = '주작의대장간', x1 = 9, x2 = 10, y = 17 },
    ['현무의성'] = { map = '현무의대장간', x1 = 9, x2 = 10, y = 17 },
    ['청룡의성'] = { map = '청룡의대장간', x1 = 9, x2 = 10, y = 17 },
    ['백호의성'] = { map = '백호의대장간', x1 = 9, x2 = 10, y = 17 },
}

local function warp_by_table(me, table)
    local map = me:map()
    if map == nil then
        return
    end
    local rule = table[map:model():name()]
    if rule == nil then
        return
    end
    local dest = name2map(rule.map)
    if dest == nil then
        return
    end
    me:map(dest, math.random(rule.x1, rule.x2), rule.y)
end

local function siege_active_on(me)
    local map = me:map()
    if map == nil then
        return false
    end
    local _, divine_beast = castle_lib.totem_of_map(map)
    if divine_beast == nil then
        return false
    end
    local castle_obj = castle(divine_beast)
    return castle_obj ~= nil and castle_obj:siege_active()
end

local function block_if_siege(me, message)
    if not siege_active_on(me) then
        return false
    end

    me:message(message or '[공성전] 지나갈 수 없습니다.', MESSAGE_TYPE.NOTIFY)
    local map = me:map()
    local x, y = me:position()
    me:map(map, x, y + 2)
    return true
end

local function owner_or_ally(me)
    local map = me:map()
    local _, divine_beast = castle_lib.totem_of_map(map)
    if divine_beast == nil then
        return false, false
    end
    local castle_obj = castle(divine_beast)
    local owner_clan_id = castle_obj and castle_obj:owner_clan() or nil
    local clan = me:clan()
    if owner_clan_id == nil or clan == nil then
        return false, false
    end
    return clan:id() == owner_clan_id, clan:is_allied(owner_clan_id)
end

return {
    on_head_to_eye = function(me)
        if block_if_siege(me) then
            return
        end
        warp_by_table(me, HEAD_TO_EYE)
    end,

    on_eye_to_head = function(me)
        if block_if_siege(me) then
            return
        end
        warp_by_table(me, EYE_TO_HEAD)
    end,

    on_heart_deney = function(me)
        if block_if_siege(me, '[공성전] 머리로 갈 수 없습니다.') then
            return
        end
        warp_by_table(me, HEART_DENEY)
    end,

    on_heart_out = function(me)
        warp_by_table(me, HEART_OUT)
    end,

    on_sewer_in = function(me)
        if block_if_siege(me, '[공성전] 지하수로로 갈 수 없습니다.') then
            return
        end

        local is_owner, is_ally = owner_or_ally(me)
        if is_ally then
            local npc = name2npc('문파성선생')
            local button = me:dialog(npc, '동맹 문파 자격으로 지하수로에 입장합니다.', { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            warp_by_table(me, SEWER_IN)
        elseif is_owner then
            if me:level() < 99 then
                me:message('성주는 99레벨 이상만 지하수로에 들어갈 수 있습니다.', MESSAGE_TYPE.NOTIFY)
                local map = me:map()
                local x, y = me:position()
                me:map(map, x, y + 2)
            else
                warp_by_table(me, SEWER_IN)
            end
        else
            me:message('성주나 동맹만 지하수로에 들어갈 수 있습니다.', MESSAGE_TYPE.NOTIFY)
            local map = me:map()
            local x, y = me:position()
            me:map(map, x, y + 2)
        end
    end,

    on_sewer_out = function(me)
        warp_by_table(me, SEWER_OUT)
    end,

    on_eight_room = function(me)
        local is_owner = owner_or_ally(me)
        if not is_owner or me:level() < 99 then
            me:message('성주(99레벨)만 팔괘방에 들어갈 수 있습니다.', MESSAGE_TYPE.NOTIFY)
            local map = me:map()
            local x, y = me:position()
            me:map(map, x, y + 2)
            return
        end
        warp_by_table(me, PALGWAE)
    end,

    on_palgwae = function(me)
        if block_if_siege(me, '[공성전] 팔괘방으로 갈 수 없습니다.') then
            return
        end

        local map = me:map()
        local rule = PALGWAE[map and map:model():name()]
        local dest = rule and name2map(rule.map) or nil
        if dest == nil then
            return
        end
        if not castle_lib.can_occupy(me, dest) then
            me:message('성주나 동맹만 팔괘방에 들어갈 수 있습니다.', MESSAGE_TYPE.NOTIFY)
            local x, y = me:position()
            me:map(map, x, y + 2)
            return
        end
        me:map(dest, math.random(rule.x1, rule.x2), rule.y)
    end,

    on_forge = function(me)
        if block_if_siege(me, '[공성전] 대장간으로 갈 수 없습니다.') then
            return
        end
        warp_by_table(me, FORGE)
    end,
}
