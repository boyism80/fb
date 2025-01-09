function on_cast(me, spell, you)
    if not you:is(OBJECT_TYPE_CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return
    end

    local mp = 10000
    if me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return
    end
    me:mp_down(mp)

    you:heal(30000)
    you:effect(165)
    you:sound(505)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    if me ~= you then
        you:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
    end
end