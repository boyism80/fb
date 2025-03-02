-- 활력 캐스팅
function ON_CAST_4024(me, you, spell)
    local mp = 30
    local sound = 63
    local effect = 22
    if spell_cast(me, you, spell, mp, sound, effect) then
        you:unbuff('마비')
    end
end