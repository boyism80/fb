-- Castle (신수성) script warps migrated from trash db_npc warp helpers.

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

local HEART_OUT = {
    ['주작의심장'] = { map = '주작의성', x1 = 72, x2 = 75, y = 50 },
    ['현무의심장'] = { map = '현무의성', x1 = 72, x2 = 75, y = 50 },
    ['청룡의심장'] = { map = '청룡의성', x1 = 72, x2 = 75, y = 50 },
    ['백호의심장'] = { map = '백호의성', x1 = 72, x2 = 75, y = 50 },
}

local function warp_by_table(me, table)
    local map = me:map()
    if map == nil then
        return
    end
    local rule = table[map:name()]
    if rule == nil then
        return
    end
    local dest = name2map(rule.map)
    if dest == nil then
        return
    end
    me:map(dest, math.random(rule.x1, rule.x2), rule.y)
end

local function block_if_siege(me)
    local map = me:map()
    if map == nil then
        return false
    end
    local divine_beast = map:model():siege_castle()
    if divine_beast == nil then
        return false
    end
    local castle_obj = castle(divine_beast)
    if castle_obj == nil or castle_obj:siege_active() == false then
        return false
    end

    me:message('[공성전] 지나갈 수 없습니다.', MESSAGE_TYPE.NOTIFY)
    local x, y = me:position()
    me:map(map, x, y + 2)
    return true
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

    on_heart_out = function(me)
        warp_by_table(me, HEART_OUT)
    end,
}
