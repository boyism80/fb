function on_cast(me, spell)
    local damage = 510
    local mp = 250
    local sound = 41
    local effect = 46
    spell_damage_near(me, spell, damage, mp, sound, effect)
end