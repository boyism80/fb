-- spell: 월아일격
-- 월아일격 캐스팅
function ON_CAST_10004(me, you, spell)
    you:sound(8)
    you:effect(120)
    if you ~= nil then
        if you:is(OBJECT_TYPE.CHARACTER) then
            you:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name(), '을', '를')))
        else
            you:message(string.format('%s %s 가합니다.', name_with(me:name(), '이', '가'), name_with(spell:name(), '을', '를')))
        end
    end
    you:damage(math.random(30000, 35000), me, { critical = false, rate = me:skill_damage_rate() / 1000.0, physical = false })
end
