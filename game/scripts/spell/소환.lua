function lookup(map, x, y, direction)
    math.randomseed(seed())
    local rand_x = nil
    local rand_y = nil
    for i = 0, 3 do
        local case = (direction + i) % 4
        if case == DIRECTION_LEFT then
            rand_x = -1
            rand_y = 0
        elseif case == DIRECTION_TOP then
            rand_x = 0
            rand_y = -1
        elseif case == DIRECTION_RIGHT then
            rand_x = 1
            rand_y = 0
        else
            rand_x = 0
            rand_y = 1
        end

        if map:movable(x+rand_x, y+rand_y) then
            return x+rand_x, y+rand_y
        end
    end

    return x, y
end


-- 소환 캐스팅
function ON_CAST_4(me, spell, name)

    if me:name() == name then
        me:message('나 자신을 소환할 수 없습니다.')
        return
    end

    local map = me:map()
    if map == nil then
        me:message('소환할 수 없는 공간입니다.')
        return
    end

    local x, y = me:position()
    local direction = me:direction()
    local my_level = me:level()
    local ch = name2ch(name)
    if ch == nil then
        me:message(string.format('%s님은 현재 바람의나라에 없습니다.', name))
        return
    end
    if ch:level() > my_level then
        me:message('마력이 미치지 않습니다.')
        return
    end

    local sound = 36
    local effect = 3
    local mp = 30
    if spell_cast(me, nil, nil, mp) then
        local new_x, new_y = lookup(map, x, y, direction)
        ch:map(map, new_x, new_y)
        me:action(ACTION_CAST_SPELL, DURATION_SPELL)
        me:sound(sound)
        me:effect(effect)
        me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    end
end