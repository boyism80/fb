-- 누리의힘 캐스팅
function ON_CAST_2010(me, spell)
    local mp = 30
    local sound = 21
    local effect = 11
    local buff_time = 180
    if buff_cast(me, me, spell, {mp=mp, sound=sound, effect=effect}) then
        me:buff(spell, buff_time)
    end
end

-- 누리의힘 버프 효과
function ON_BUFF_2010(me, spell)
    me:buff_str(me:buff_str()+3)
end

-- 누리의힘 버프 해제 효과
function ON_UNBUFF_2010(me, spell)
    me:buff_str(me:buff_str()-3)
end