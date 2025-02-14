function on_cast(me, you, spell)
    local hp = 30000
    local mp = 10000
    local sound = 505
    local effect = 165
    spell_heal(me, you, spell, hp, mp, sound, effect)
end