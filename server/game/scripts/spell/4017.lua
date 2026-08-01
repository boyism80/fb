-- spell: 신령지익진
-- 신령지익진 버프 효과

-- 신령지익진 버프 해제 효과

-- 신령지익진 지속 효과

return {
    -- on_cast = function(me, you, sp)
    -- end,

    on_buff = function(me, spell)
    	me:damage_derate(me:damage_derate() + 1000)
    end,

    on_unbuff = function(me, spell)
    	me:damage_derate(me:damage_derate() - 1000)
    end,

    -- on_concast = function(me, sp)
    -- end
}
