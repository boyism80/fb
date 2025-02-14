function on_cast(me, you, spell)
    local mp = 30
    local sound = 63
    local effect = 22
    if spell_cast(me, you, spell, mp, sound, effect) then
        you:unbuff('중독')
    end
end