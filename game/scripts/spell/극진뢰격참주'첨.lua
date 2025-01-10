function on_cast(me, spell)
    local damage = 3560
    local mp = 470
    local sound = 59
    local effect = 30
    spell_damage_near(me, spell, damage, mp, sound, effect)
end