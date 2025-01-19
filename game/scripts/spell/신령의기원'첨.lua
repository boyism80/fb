function on_cast(me, spell)
    local mp = 1000
    local sound = 97
    local effect = 63
    local hp = 10000
    if not spell_heal_group(me, spell, hp, mp, sound, effect) then
        return
    end
end