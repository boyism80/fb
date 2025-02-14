function on_cast(me, you, spell)
    local hp = 1000
    local mp = 960
    local sound = 76
    local effect = 66
    spell_heal_near(me, you, spell, hp, mp, sound, effect)
end