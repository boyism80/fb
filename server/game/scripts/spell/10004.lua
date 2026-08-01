-- spell: 월해협격
-- 월해협격 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        if not spell.assert_map_damage(me, you) then
            return
        end

        you:sound(8)
        you:effect(120)
        if you ~= nil then
            if you:is(OBJECT_TYPE.CHARACTER) then
                you:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(sp:name(), '을', '를')))
            else
                you:message(string.format('%s %s 가합니다.', name_with(me:name(), '이', '가'), name_with(sp:name(), '을', '를')))
            end
        end
        me:damage_to(you, math.random(30000, 35000), { critical = false, rate = me:skill_damage_rate() / 1000.0, physical = false })
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
