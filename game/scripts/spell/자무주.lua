function on_cast(me, you, spell)
    local damage = 320
    local mp = 120
    local sound = 45
    local effect = 23
    spell_damage(me, you, spell, damage, mp, sound, effect)
end