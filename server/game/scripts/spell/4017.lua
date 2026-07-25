-- spell: 신령지익진
-- 신령지익진 버프 효과

-- 신령지익진 버프 해제 효과

-- 신령지익진 지속 효과

return {
    -- ON_CAST = function(me, you, sp)
    -- end,

    ON_BUFF = function(me, spell)
    	me:damage_derate(me:damage_derate() + 1000)
    end,

    ON_UNBUFF = function(me, spell)
    	me:damage_derate(me:damage_derate() - 1000)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
