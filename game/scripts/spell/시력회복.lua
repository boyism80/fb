function on_cast(me, you, spell)
    local mp = 40
    local sound = 75
    local effect = 10
    if spell_cast(me, you, spell, mp, sound, effect) then
        you:unbuff('절망')
    end
end