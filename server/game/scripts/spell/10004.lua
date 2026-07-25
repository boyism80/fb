-- spell: 월아일격
-- 월아일격 캐스팅

return {
    ON_CAST = function(me, you, spell)
        you:sound(8)
        you:effect(120)
        if you ~= nil then
            if you:is(OBJECT_TYPE.CHARACTER) then
                you:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name(), '을', '를')))
            else
                you:message(string.format('%s %s 가합니다.', name_with(me:name(), '이', '가'), name_with(spell:name(), '을', '를')))
            end
        end
        me:damage_to(you, math.random(30000, 35000), { critical = false, rate = me:skill_damage_rate() / 1000.0, physical = false })
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
