function on_cast(me, spell)
    local mp = 30
    if me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return
    end
    me:mp_down(mp)

    me:heal(50)
    me:sound(3)
    me:effect(5)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
end