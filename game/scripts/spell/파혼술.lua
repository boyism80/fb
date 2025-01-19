function on_cast(me, you, spell)
    local mp = 30
    local sound = 77
    local effect = 49
    if spell_cast(me, you, spell, mp, sound, effect) then
        you:unbuff('혼마술')
    end
end