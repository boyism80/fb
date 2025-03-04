-- 대지의힘 캐스팅
function ON_CAST_3001(me, you, spell)
    if not you:is(OBJECT_TYPE_CHARACTER) then
        return me:message('걸리지 않습니다.')
    end

    local mp = 30
    local sound = 21
    local effect = 11
    local buff_time = 5
    if buff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time, me)
    end
end

-- 대지의힘 버프 효과
function ON_BUFF_3001(me, spell)
    me:buff_str(me:buff_str()+3)
end

-- 대지의힘 버프 해제 효과
function ON_UNBUFF_3001(me, spell)
    me:buff_str(me:buff_str()-3)
end