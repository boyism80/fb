function on_cast(me, you, spell)
    local hp = 200
    local mp = 200
    local sound = 34
    local effect = 66
    spell_heal_near(me, you, spell, hp, mp, sound, effect)
end