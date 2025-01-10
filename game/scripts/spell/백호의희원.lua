function on_cast(me, you, spell)
    local hp = me:mp()*2
    local mp = me:mp()//2
    local sound = 95
    local effect = 64
    spell_heal(me, you, spell, hp, mp, sound, effect)
end