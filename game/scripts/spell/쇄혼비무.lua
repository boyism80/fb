function on_cast(me, spell)
    local mp = 140
    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    local nears = me:nears(0xFF & ~OBJECT_TYPE_ITEM)
    local x, y = me:position()
    local direction = me:direction()
    for i = 1, 3 do
        if front_obj(x, y, direction, i, nears) ~= nil then
            return failed_attack_spell(me)
        end
    end

    local front = front_obj(x, y, direction, 4, nears)
    if front == nil then
        return failed_attack_spell(me)
    end

    local hp_consume = (me:hp() * 2) // 3
    me:hp(math.max(1, me:hp() - hp_consume))

    if direction == DIRECTION_LEFT then
        x = x-3
    elseif direction == DIRECTION_RIGHT then
        x = x+3
    elseif direction == DIRECTION_TOP then
        y = y-3
    else
        y = y+3
    end 
    me:position(x, y)

    local effect = 97
    local sound = 100
    attack_cast(me, front, spell, mp, sound, effect)

    if front:is(OBJECT_TYPE_LIFE) then
        local rate = me:skill_damage_rate() / 1000.0
        local damage = math.floor(me:hp() * 3 * rate)
        front:damage(damage, me)
    end
end

function on_cast(me, spell)
    local map = me:map()
    if map == nil then
        return
    end

    local hp = me:hp()*2 // 3
    local mp = 140
    local effect = 97
    local sound = 100
    local damage = me:hp() * 3
    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    for i = 1, 3 do
        if not map:movable(me, i) then
            return failed_attack_spell(me)
        end
    end

    local nears = me:nears(0xFF & ~OBJECT_TYPE_ITEM)
    local x, y = me:position()
    local direction = me:direction()
    local front = front_obj(x, y, direction, 4, nears)
    if front == nil then
        return failed_attack_spell(me)
    end

    local hp_consume = (me:hp() * 2) // 3
    me:hp(math.max(1, me:hp() - hp_consume))

    if direction == DIRECTION_LEFT then
        x = x-3
    elseif direction == DIRECTION_RIGHT then
        x = x+3
    elseif direction == DIRECTION_TOP then
        y = y-3
    else
        y = y+3
    end 
    me:position(x, y)

    attack_cast(me, front, spell, hp, mp, damage, '쇄혼비무', sound, effect)
end