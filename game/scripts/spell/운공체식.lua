function on_cast(me, spell)
    local hp = 10000
    local mp = 10000
    local sound = 61
    local effect = 166
    spell_heal(me, me, spell, hp, mp, sound, effect)
end