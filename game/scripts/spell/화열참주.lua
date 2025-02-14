function on_cast(me, you, spell)
    local damage = 740
    local mp = 250
    local sound = 42
    local effect = 47
    spell_damage(me, you, spell, damage, mp, sound, effect)
end