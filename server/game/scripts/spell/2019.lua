-- spell: 이기어검술
-- 이기어검술 캐스팅
-- function ON_CAST_2019(me, you, spell)
--
-- end

-- 이기어검술 버프 효과
function ON_BUFF_2019(me, spell)
    me:buff_phydef(me:buff_phydef() - 30)
end

-- 이기어검술 버프 해제 효과
function ON_UNBUFF_2019(me, spell)
    me:buff_phydef(me:buff_phydef() - 30)
end
