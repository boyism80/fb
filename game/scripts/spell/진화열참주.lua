function on_cast(me, you, spell)
    local damage = 1950
    local mp = 330
    local sound = 43
    local effect = 48
    spell_damage(me, you, spell, damage, mp, sound, effect)
end