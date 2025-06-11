-- 심안투영 캐스팅
function ON_CAST_4008(me, spell)
    local mp = 30
    local sound = 78
    local effect = 11
    local buff_time = 600
    if buff_cast(me, me, spell, {mp=mp, sound=sound, effect=effect}) then
        me:buff(spell, buff_time)
    end
end

-- 심안투영 버프 효과
function ON_BUFF_4008(me, spell)
    me:detect(true)
end

-- 심안투영 버프 해제 효과
function ON_UNBUFF_4008(me, spell)
    me:detect(false)

end