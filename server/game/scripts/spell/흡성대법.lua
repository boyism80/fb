-- 흡성대법 캐스팅
function ON_CAST_2017(me, you, spell)
    local damage = 3000
    local mp = 60
    local sound = 38
    local effect = 44

    if not you:is(OBJECT_TYPE.MOB) then
    	return me:message('걸리지 않습니다.')
    end

    if you:maxhp() > damage then
    	return me:message('걸리지 않습니다.')
    end
    
    me:heal(math.min(damage, you:hp()))
    spell_damage(me, you, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end