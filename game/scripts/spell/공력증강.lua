function on_cast(me, spell)
    local mp = 30
    local effect = 11
    local sound = 65

    if not spell_cast(me, me, spell, mp, sound, effect) then
        return
    end

    local success = math.random() > 0.5
    if not success then
        return me:message('정신집중에 실패하였습니다.')
    else
        me:mp(me:base_mp())
        me:hp(math.max(100, me:hp() - math.floor(me:base_mp() * 0.4)))
    end
end