function on_cast(me, you, spell)
    local damage = 530
    local mp = 180
    local sound = 46
    local effect = 24
    spell_damage(me, you, spell, damage, mp, sound, effect)
end