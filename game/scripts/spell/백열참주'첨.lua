function on_cast(me, spell)
    local damage = 720
    local mp = 330
    local sound = 52
    local effect = 42
    spell_damage_near(me, spell, damage, mp, sound, effect)
end