-- 마비 캐스팅
function ON_CAST_3052(me, you, spell)
    local mp = 80
    local sound = 70
    local effect = 1
    local buff_time = 20
    if debuff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time, me)
    end
end

-- 마비 버프 효과
function ON_BUFF_3052(me, spell)
    me:paralysis(true)
end

-- 마비 버프 해제 효과
function ON_UNBUFF_3052(me, spell)

    me:paralysis(false)
end