-- 차폐 캐스팅
function ON_CAST_4005(me, spell)
    local mp = 100
    local sound = 8
    local effect = 2
    local buff_time = 21
    if buff_cast(me, me, spell, {mp=mp, sound=sound, effect=effect}) then
        me:buff(spell, buff_time)
    end
end

-- 차폐 버프 효과
function ON_BUFF_4005(me, spell)
    me:cover(true)
end

-- 차폐 버프 해제 효과
function ON_UNBUFF_4005(me, spell)
    me:cover(false)

end