-- 퇴마주 캐스팅
function ON_CAST_4022(me, you, spell)
    local mp = 30
    local sound = 67
    local effect = 22
    if spell_cast(me, you, spell, {mp=mp, sound=sound, effect=effect}) then
        you:unbuff('저주')
    end
end