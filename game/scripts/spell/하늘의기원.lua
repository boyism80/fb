function on_cast(me, spell)
    if not you:is(OBJECT_TYPE_CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return
    end
    
    local mp = 120
    if me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return
    end
    me:mp_down(mp)

    me:heal(200)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    me:effect(5)
    me:sound(3)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
end