-- 파혼술 캐스팅
function ON_CAST_4007(me, you, spell)
    local mp = 30
    local sound = 77
    local effect = 49
    if spell_cast(me, you, spell, {mp=mp, sound=sound, effect=effect}) then
        you:unbuff('혼마술')
    end
end