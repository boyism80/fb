function on_cast(me, you, spell)
    if not you:is(OBJECT_TYPE_MOB) then
        return me:message('걸리지 않습니다.')
    end

    local mp = 50
    local sound = 2
    local effect = 88
    if spell_cast(me, you, spell, mp, sound, effect) then
        you:target(me)
    end
end