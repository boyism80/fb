-- 투명 캐스팅
function ON_CAST_2012(me, spell)
    local mp = 0
    local sound = 28
    local effect = 20
    local buff_time = 180
    if buff_cast(me, me, spell, {mp=mp, sound=sound, effect=effect}) then
        me:state(STATE_HALF_CLOACK)
        me:buff(spell, buff_time)
    end
end

-- 투명 버프 효과
function ON_BUFF_2012(me, spell)

end

-- 투명 버프 해제 효과
function ON_UNBUFF_2012(me, spell)
    me:state(STATE_NORMAL)
end