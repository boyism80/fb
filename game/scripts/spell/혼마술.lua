-- 혼마술 캐스팅
function ON_CAST_4009(me, you, spell)
    local mp = 40
    local sound = 79
    local effect = 39
    local buff_time = 425
    if debuff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time, me)
    end
end

-- 혼마술 버프 효과
function ON_BUFF_4009(me, spell)
    me:buff_phydef(me:buff_phydef() + 50)
end

-- 혼마술 버프 해제 효과
function ON_UNBUFF_4009(me, spell)
    me:buff_phydef(me:buff_phydef() - 50)

end