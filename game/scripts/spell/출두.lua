function lookup(map, x, y)
    math.randomseed(seed())
    local offset = math.random(0, 3)
    local rand_x = nil
    local rand_y = nil
    local direction = nil
    for i = 0, 3 do
        local case = (offset + i) % 4
        if case == 0 then
            rand_x = 0
            rand_y = -1
            direction = DIRECTION_BOTTOM
        elseif case == 1 then
            rand_x = 0
            rand_y = 1
            direction = DIRECTION_TOP
        elseif case == 2 then
            rand_x = -1
            rand_y = 0
            direction = DIRECTION_RIGHT
        else
            rand_x = 1
            rand_y = 0
            direction = DIRECTION_LEFT
        end

        if map:movable(x+rand_x, y+rand_y) then
            return x+rand_x, y+rand_y, direction
        end
    end

    return x, y, DIRECTION_BOTTOM
end

function on_cast(me, spell, name)

    if me:name() == name then
        return me:message('나 자신을 출두할 수 없습니다.')
    end
    local my_level = me:level()
    local ch = name2ch(name)
    if ch == nil then
        return me:message(string.format('%s님은 현재 바람의나라에 없습니다.', name))
    end
    if ch:level() > my_level then
        return me:message('마력이 미치지 않습니다.')
    end

    local map = ch:map()
    if map == nil then
        return me:message('이동할 수 없는 공간입니다.')
    end

    local sound = 29
    local effect = 16
    local mp = 30
    if spell_cast(me, nil, nil, mp) then
        local x, y = ch:position()
        local new_x, new_y, direction = lookup(map, x, y)
        me:map(map, new_x, new_y)
        me:direction(direction)
        me:sound(sound)
        me:effect(effect)
        me:message(string.format('%s 외웠습니다.', name_with(spell:model():name())))
    end
end