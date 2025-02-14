function on_cast(me, you, spell)
    local damage = 3580 -- + me:int()*5
    local mp = 360
    local sound = 44
    local effect = 48
    spell_damage(me, you, spell, damage, mp, sound, effect)
end