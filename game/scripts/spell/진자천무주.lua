function on_cast(me, you, spell)
    local damage = 1950
    local mp = 330
    local sound = 48
    local effect = 26
    spell_damage(me, you, spell, damage, mp, sound, effect)
end