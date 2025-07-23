-- 신령지익진 버프 효과
function ON_BUFF_4017(me, spell)
	me:damage_derate(me:damage_derate() + 1000)
end

-- 신령지익진 버프 해제 효과
function ON_UNBUFF_4017(me, spell)
	me:damage_derate(me:damage_derate() - 1000)
end

-- 신령지익진 지속 효과
function ON_CONCAST_4017(me, caster, buff)

end