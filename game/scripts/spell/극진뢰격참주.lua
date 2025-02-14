function on_cast(me, you, spell)
    local damage = 3580 -- + me:int()*5
    local mp = 360
    local sound = 59
    local effect = 30
    spell_damage(me, you, spell, damage, mp, sound, effect)
end