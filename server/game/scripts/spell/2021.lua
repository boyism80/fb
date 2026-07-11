-- spell: 무형술
-- 무형술 캐스팅
-- function ON_CAST_2021(me, you, spell)
--
-- end

-- 무형술 버프 효과
function ON_BUFF_2021(me, spell)
    me:buff_phydef(me:buff_phydef() - 40)
end

-- 무형술 버프 해제 효과
function ON_UNBUFF_2021(me, spell)
    me:buff_phydef(me:buff_phydef() - 40)
end
