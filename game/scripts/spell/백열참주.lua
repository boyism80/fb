function on_cast(me, you, spell)
    local damage = 740
    local mp = 250
    local sound = 52
    local effect = 42
    spell_damage(me, you, spell, damage, mp, sound, effect)
end