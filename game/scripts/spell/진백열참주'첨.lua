function on_cast(me, spell)
    local damage = 1930
    local mp = 400
    local sound = 53
    local effect = 43
    spell_damage_near(me, spell, damage, mp, sound, effect)
end