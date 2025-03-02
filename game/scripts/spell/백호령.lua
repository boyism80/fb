-- 백호령 캐스팅
function ON_CAST_1008(me, spell)
    local mp = 30
    local sound = 12
    local effect = 2
    local buff_time = 600
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

-- 백호령 버프 효과
function ON_BUFF_1008(me, spell)
    me:dam(me:dam()+3)
end

-- 백호령 버프 해제 효과
function ON_UNBUFF_1008(me, spell)
    me:dam(me:dam()-3)

end