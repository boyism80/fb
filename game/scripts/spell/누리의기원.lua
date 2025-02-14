function on_cast(me, spell)
    local hp = 50
    local mp = 30
    local sound = 3
    local effect = 5
    spell_heal(me, me, spell, hp, mp, sound, effect)
    spell:delay2(100000)
end