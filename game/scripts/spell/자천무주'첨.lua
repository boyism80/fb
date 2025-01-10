function on_cast(me, spell)
    local damage = 720
    local mp = 330
    local sound = 47
    local effect = 25
    spell_damage_near(me, spell, damage, mp, sound, effect)
end