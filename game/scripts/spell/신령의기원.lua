function on_cast(me, spell, you)
    local mp = 1000
    if me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return
    end
    me:mp_down(mp)

    if you:is(OBJECT_TYPE_LIFE) then
        you:heal(10000)
    end

    you:effect(63)
    you:sound(97)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    if me ~= you and you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
    end
end