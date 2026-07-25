-- spell: 파력무참진
-- 파력무참진 버프 효과

-- 파력무참진 버프 해제 효과

-- 파력무참진 지속 효과

return {
    -- ON_CAST = function(me, you, sp)
    -- end,

    ON_BUFF = function(me, spell)
        me:damage_rate(me:damage_rate() + 1000)
        me:skill_damage_rate(me:skill_damage_rate() + 1000)
    end,

    ON_UNBUFF = function(me, spell)
    	me:damage_rate(me:damage_rate() - 1000)
        me:skill_damage_rate(me:skill_damage_rate() - 1000)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
