function spell_damage(me, you, spell, damage, mp, sound, effect)
    if not you:is(OBJECT_TYPE_LIFE) then
        return me:message('대상이 올바르지 않습니다.')
    end

    if me:mp() < mp then
        return me:message('마력이 부족합니다.')
    end

    you:effect(effect)
    you:sound(sound)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    if you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
    end
    you:damage(damage, me)
end

function spell_heal(me, you, spell, hp, mp, sound, effect)
    if not you:is(OBJECT_TYPE_CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return
    end
    
    if me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return
    end
    me:mp_down(mp)

    you:heal(hp)
    you:effect(effect)
    you:sound(sound)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    if me ~= you then
        you:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
    end
end