function on_cast(me, you, spell)
    local mp = 1000
    local sound = 97
    local effect = 63
    if not spell_cast(me, you, spell, mp, sound, effect) then
        return
    end
    
    if you:is(OBJECT_TYPE_LIFE) then
        you:heal(10000)
    end
end