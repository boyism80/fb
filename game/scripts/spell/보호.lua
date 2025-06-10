-- 보호 캐스팅
function ON_CAST_4004(me, you, spell)
    local mp = 30
    local sound = 8
    local effect = 110
    local buff_time = 180
    if buff_cast(me, you, spell, {mp = mp, sound = sound, effect = effect}) then
        you:buff(spell, buff_time, me)
    end
end

-- 보호 버프 효과
function ON_BUFF_4004(me, spell)
    me:damage_derate(me:damage_derate() + 1000)
end

-- 보호 버프 해제 효과
function ON_UNBUFF_4004(me, spell)
    me:damage_derate(me:damage_derate() - 1000)

end