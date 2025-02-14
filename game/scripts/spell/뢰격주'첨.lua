function on_cast(me, spell)
    local damage = 510
    local mp = 250
    local sound = 56
    local effect = 28
    spell_damage_near(me, spell, damage, mp, sound, effect)
end