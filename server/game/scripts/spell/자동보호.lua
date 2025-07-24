-- 자동보호 캐스팅
function ON_CAST_7(me, spell)
    if me:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        return false
    end

    me:sound(8)
    me:effect(110)
    me:buff(spell, 180, me)
end

-- 자동보호 버프 효과
function ON_BUFF_7(me, spell)
    me:damage_derate(me:damage_derate() + 1000)
end

-- 자동보호 버프 해제 효과
function ON_UNBUFF_7(me, spell)
    me:damage_derate(me:damage_derate() - 1000)

end