-- 파력무참진 버프 효과
function ON_BUFF_4019(me, spell)
    me:damage_rate(me:damage_rate() + 1000)
    me:skill_damage_rate(me:skill_damage_rate() + 1000)
end

-- 파력무참진 버프 해제 효과
function ON_UNBUFF_4019(me, spell)
	me:damage_rate(me:damage_rate() - 1000)
    me:skill_damage_rate(me:skill_damage_rate() - 1000)
end

-- 파력무참진 지속 효과
function ON_CONCAST_4019(me, caster, buff)

end