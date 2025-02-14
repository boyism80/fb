function on_cast(me, spell)
    local damage = 1930
    local mp = 400
    local sound = 58
    local effect = 30
    spell_damage_near(me, spell, damage, mp, sound, effect)
end