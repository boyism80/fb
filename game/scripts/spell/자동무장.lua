-- 자동무장 캐스팅
function ON_CAST_8(me, spell)
    if me:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        return false
    end

    me:sound(8)
    me:effect(110)
    me:buff(spell, 180, me)
end

-- 자동무장 버프 효과
function ON_BUFF_8(me, spell)
    me:buff_phydef(me:buff_phydef() -10)
end

-- 자동무장 버프 해제 효과
function ON_UNBUFF_8(me, spell)
    me:buff_phydef(me:buff_phydef() + 10)
end