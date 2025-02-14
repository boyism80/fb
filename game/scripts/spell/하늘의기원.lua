function on_cast(me, spell)
    local hp = 200
    local mp = 120
    local sound = 3
    local effect = 5
    spell_heal(me, me, spell, hp, mp, sound, effect)
end