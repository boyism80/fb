function on_cast(me, spell)
    local damage = 510
    local mp = 250
    local sound = 46
    local effect = 24
    spell_damage_near(me, spell, damage, mp, sound, effect)
end