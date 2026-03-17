local TREASURE_SPOTS = {
    { map = '도삭산801층', x = 32, y = 4 },
    { map = '도삭산801층', x = 4, y = 5 },
    { map = '도삭산804층', x = 30, y = 11 },
    { map = '도삭산804층', x = 4, y = 17 },
    { map = '도삭산805층', x = 27, y = 4 },
    { map = '도삭산805층', x = 4, y = 7 },
    { map = '도삭산810층', x = 3, y = 20 },
    { map = '도삭산810층', x = 20, y = 4 },
    { map = '도삭산812층', x = 6, y = 33 },
    { map = '도삭산812층', x = 16, y = 3 },
}

function ON_ACTIVATED_10134(me, item)
    local quest = me:quest(QUEST_MOUNTAIN_GOD)
    if quest == nil or quest:step() < 1 then
        return
    end

    local param = quest:param() or ''

    if param ~= '' and string.match(param, '^[^,]+,%d+,%d+$') then
        local parts = {}
        for p in string.gmatch(param, '[^,]+') do
            table.insert(parts, p)
        end
        if #parts == 3 then
            me:dialog(item:model(), parts[1] .. ' ' .. parts[2] .. ', ' .. parts[3] .. '에 숨겨두다.', false, true)
            return
        end
    end

    local idx = math.random(1, #TREASURE_SPOTS)
    local spot = TREASURE_SPOTS[idx]
    local new_param = string.format('%s,%d,%d', spot.map, spot.x, spot.y)
    quest:param(new_param)
    me:dialog(item:model(), spot.map .. ' ' .. tostring(spot.x) .. ', ' .. tostring(spot.y) .. '에 숨겨두다.', false, true)
end
