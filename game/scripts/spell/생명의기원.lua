function on_cast(me, you, spell)
    local hp = 5000
    local mp = 300
    local sound = 98
    local effect = 65
    spell_heal(me, you, spell, hp, mp, sound, effect)
end