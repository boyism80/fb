function on_cast(me, you, spell)
    local damage = 280
    local mp = 80
    local sound = 8
    local effect = 86
    spell_damage(me, you, spell, damage, mp, sound, effect)
end