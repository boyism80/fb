function near(me, type)
    local result = {}
    local map = me:map()
    if map == nil then
        return result
    end

    local x, y = me:position()
    for _, obj in pairs(map:objects()) do
        if not obj:is(type) then
            goto SKIP
        end

        local obj_x, obj_y = obj:position()
        if (obj_x == x+1 or obj_x == x-1) and (obj_y == y) then
            table.insert(result, obj)
        elseif (obj_y == y+1 or obj_y == y-1) and (obj_x == x) then
            table.insert(result, obj)
        else
            
        end
        ::SKIP::
    end

    return result
end

function spell_cast(me, you, spell, mp, sound, effect)
    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    you:effect(effect)
    you:sound(sound)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    if me ~= you and you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
    end
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    return true
end

function spell_damage(me, you, spell, damage, mp, sound, effect)
    if not you:is(OBJECT_TYPE_LIFE) then
        return me:message('대상이 올바르지 않습니다.')
    end

    if me:mp() < mp then
        return me:message('마력이 부족합니다.')
    end
    me:mp_down(mp)

    you:effect(effect)
    you:sound(sound)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    if you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
    end
    you:damage(damage, me)
end

function spell_damage_near(me, spell, damage, mp, sound, effect)
    if me:mp() < mp then
        return me:message('마력이 부족합니다.')
    end
    me:mp_down(mp)

    me:sound(sound)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    for _, you in pairs(near(me, OBJECT_TYPE_LIFE)) do
        you:effect(effect)
        if you:is(OBJECT_TYPE_CHARACTER) then
            you:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
        end
        you:damage(damage, me)
    end
end

function spell_heal(me, you, spell, hp, mp, sound, effect)
    if not you:is(OBJECT_TYPE_CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return
    end

    if not spell_cast(me, you, spell, mp, sound, effect) then
        return
    end

    you:heal(hp)
end

function spell_heal_near(me, you, spell, hp, mp, sound, effect)
    if not you:is(OBJECT_TYPE_CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return
    end
    
    if me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return
    end
    me:mp_down(mp)

    local targets = near(you, OBJECT_TYPE_CHARACTER)
    table.insert(targets, you)

    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    for _, ch in pairs(targets) do
        ch:heal(hp)
        ch:effect(effect)
        ch:sound(sound)
        if me ~= ch then
            ch:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
        end
    end
end

function spell_disguise_look(me, mobs, name)
    for k, v in pairs(mobs) do
        if k == name then
            return v
        end
    end

    me:message('변신 가능한 동물')
    for k, v in pairs(mobs) do
        me:message(k)
    end
    return nil
end

function spell_disguise(me, mobs, name, spell, mp, sound, effect, buff_time)
    local look = spell_disguise_look(me, mobs, name)
    if look == nil then
        return
    end

    if spell_cast(me, me, spell, mp, sound, effect) then
        me:disguise(look)
        me:buff(spell, buff_time)
    end
end

function unbuff(me, spell)
    me:message(string.format('%s 해제', spell:name()))
end