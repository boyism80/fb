function on_cast(me, spell)
    local mp = 30
    if me:mp() < mp then
        return me:message('마력이 부족합니다.')
    end
    me:mp_down(mp)

    local success = math.random() > 0.5
    if not success then
        return me:message('정신집중에 실패하였습니다.')
    end

    me:mp(me:base_mp())
    me:hp(math.max(100, me:hp() - math.floor(me:base_mp() * 0.4)))
    me:effect(11)
    me:sound(65)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
end