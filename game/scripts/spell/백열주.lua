function on_cast(me, you, spell)
    local damage = 320
    local mp = 120
    local sound = 50
    local effect = 40
    spell_damage(me, you, spell, damage, mp, sound, effect)
end