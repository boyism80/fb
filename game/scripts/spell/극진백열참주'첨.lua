function on_cast(me, spell)
    local damage = 3560
    local mp = 470
    local sound = 54
    local effect = 43
    spell_damage_near(me, spell, damage, mp, sound, effect)
end