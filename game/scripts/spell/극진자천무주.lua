function on_cast(me, you, spell)
    local damage = 3580 -- + me:int()*5
    local mp = 360
    local sound = 49
    local effect = 26
    spell_damage(me, you, spell, damage, mp, sound, effect)
end