-- 시력회복 캐스팅
function ON_CAST_4006(me, you, spell)
    local mp = 40
    local sound = 75
    local effect = 10
    if spell_cast(me, you, spell, {mp=mp, sound=sound, effect=effect}) then
        you:unbuff('절망')
    end
end