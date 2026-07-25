-- spell: 무형술
-- 무형술 캐스팅
-- 무형술 버프 효과

-- 무형술 버프 해제 효과

return {
    -- ON_CAST = function(me, you, sp)
    -- end,

    ON_BUFF = function(me, spell)
        me:buff_phydef(me:buff_phydef() - 40)
    end,

    ON_UNBUFF = function(me, spell)
        me:buff_phydef(me:buff_phydef() - 40)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
