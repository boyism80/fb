function on_cast(me, spell)
    local damage = 1930
    local mp = 400
    local sound = 43
    local effect = 48
    spell_damage_near(me, spell, damage, mp, sound, effect)
end