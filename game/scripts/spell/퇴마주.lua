function on_cast(me, you, spell)
    local mp = 30
    local sound = 67
    local effect = 22
    if spell_cast(me, you, spell, mp, sound, effect) then
        you:unbuff('저주')
    end
end