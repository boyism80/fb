-- 소혼강신 캐스팅
function ON_CAST_4001(me, spell, message)
    local map = me:map()
    if map == nil then
        return
    end

    local spawnables = {
        ['평웅'] = 68,
        ['불여우'] = 75,
        ['자호'] = 80,
        ['해골'] = 90
    }

    local required_lv = spawnables[message]
    local level = me:level()
    if required_lv == nil or required_lv > level then
        me:message('가능한 동물')
        for name, lv in pairs(spawnables) do
            if level >= lv then
                me:message(name)
            end
        end
        return
    end

    local spawned_mobs = me:spawned_mobs()
    if #spawned_mobs >= 8 then
        return me:message('더 이상 소환할 수 없습니다.')
    end

    local mp = 30
    local sound = 81
    local effect = nil
    if not spell_cast(me, me, spell, mp, sound, effect) then
        return
    end

    local x, y = me:position()
    local direction = DIRECTION_BOTTOM
    if x > 0 and map:movable(me, x-1, y) then
        x = x - 1
        direction = DIRECTION_RIGHT
    elseif x < map:width() and map:movable(me, x+1, y) then
        x = x + 1
        direction = DIRECTION_LEFT
    elseif y > 0 and map:movable(me, x, y-1) then
        y = y - 1
        direction = DIRECTION_BOTTOM
    elseif y < map:width() and map:movable(me, x, y+1) then
        y = y + 1
        direction = DIRECTION_TOP
    end
    local mob = me:spawn_mob(message, x, y)
    mob:direction(direction)
end