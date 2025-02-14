function on_cast(me, you, spell)
    if not you:is(OBJECT_TYPE_LIFE) then
        return me:message('걸리지 않습니다.')
    end

    local mp = me:mp()
    local sound = 80
    local effect = 18
    if spell_cast(me, you, spell, mp, sound, effect) then
        you:mp(math.min(you:maxmp(), you:mp() + mp))
    end
end