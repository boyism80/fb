-- 자동투명 캐스팅
function ON_CAST_13(me, spell)
    local buff_time = 180
    if me:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        return false
    end

    me:buff(spell, buff_time)
end

-- 자동투명 버프 효과
function ON_BUFF_13(me, spell)
    me:state(STATE.HALF_CLOACK)
end

-- 자동투명 버프 해제 효과
function ON_UNBUFF_13(me, spell)
    me:state(STATE.NORMAL)

end