function on_cast(me, spell)
    local damage = 300
    local mp = 180
    local sound = 45
    local effect = 23
    spell_damage_near(me, spell, damage, mp, sound, effect)
end