function on_cast(me, you, spell)
    local damage = 1950
    local mp = 330
    local sound = 58
    local effect = 30
    spell_damage(me, you, spell, damage, mp, sound, effect)
end