function on_cast(me, spell)
    local hp = me:mp()*2
    local mp = me:mp()//2
    local sound = 95
    local effect = 64
    if not spell_heal_group(me, spell, hp, mp, sound, effect) then
        return
    end
end