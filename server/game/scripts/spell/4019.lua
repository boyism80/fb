-- spell: 파력무참진
-- 파력무참진 버프 효과

-- 파력무참진 버프 해제 효과

-- 파력무참진 지속 효과

return {
    -- on_cast = function(me, you, sp)
    -- end,

    on_buff = function(me, spell)
        me:damage_rate(me:damage_rate() + 1000)
        me:skill_damage_rate(me:skill_damage_rate() + 1000)
    end,

    on_unbuff = function(me, spell)
    	me:damage_rate(me:damage_rate() - 1000)
        me:skill_damage_rate(me:skill_damage_rate() - 1000)
    end,

    -- on_concast = function(me, sp)
    -- end
}
