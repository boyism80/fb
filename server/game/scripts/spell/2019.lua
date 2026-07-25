-- spell: 이기어검술
-- 이기어검술 캐스팅
-- 이기어검술 버프 효과

-- 이기어검술 버프 해제 효과

return {
    -- on_cast = function(me, you, sp)
    -- end,

    on_buff = function(me, spell)
        me:buff_phydef(me:buff_phydef() - 30)
    end,

    on_unbuff = function(me, spell)
        me:buff_phydef(me:buff_phydef() - 30)
    end,

    -- on_concast = function(me, sp)
    -- end
}
