function on_cast(me, you, spell)
    local damage = 1500
    local mp = 580
    local sound = 82
    local effect = 17
    spell_damage_near_target(me, you, spell, damage, mp, sound, effect)
end