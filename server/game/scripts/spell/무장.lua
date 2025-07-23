-- 무장 캐스팅
function ON_CAST_4003(me, you, spell)
    local mp = 30
    local sound = 8
    local effect = 110
    local buff_time = 180
    if buff_cast(me, you, spell, {mp = mp, sound = sound, effect = effect}) then
        you:buff(spell, buff_time, me)
    end
end

-- 무장 버프 효과
function ON_BUFF_4003(me, spell)
    me:buff_phydef(me:buff_phydef() -10)
end

-- 무장 버프 해제 효과
function ON_UNBUFF_4003(me, spell)
    me:buff_phydef(me:buff_phydef() + 10)
end