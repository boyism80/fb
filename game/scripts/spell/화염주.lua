function on_cast(me, you, spell)
    local damage = 320
    local mp = 120
    local sound = 40
    local effect = 45
    spell_damage(me, you, spell, damage, mp, sound, effect)
end