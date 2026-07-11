-- spell: 파력무참
-- 파력무참 캐스팅
local spell = require('lib.spell')

function ON_CAST_4018(me, sp)
    local mp = 0
    local sound = 108
    local effect = 115
    local buff_time = 45
    if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
        me:buff(sp, buff_time, me)
    end
end

-- 파력무참 버프 효과
-- function ON_BUFF_4018(me, sp)
--
-- end

-- 파력무참 버프 해제 효과
-- function ON_UNBUFF_4018(me, sp)
--
-- end

-- 파력무참 지속 효과
function ON_CONCAST_4018(me, caster, buff)
    local x, y = me:position()
    local begin_x = x-3
    local begin_y = y-3
    local end_x = x+3
    local end_y = y+3
    local targets = {}
    local sp = name2spell('파력무참진')
    for _, obj in pairs(me:nears(OBJECT_TYPE.LIFE)) do
        local obj_x, obj_y = obj:position()
        if not obj:isbuff(sp) and obj_x >= begin_x and obj_x <= end_x and obj_y >= begin_y and obj_y <= end_y then
            table.insert(targets, obj)
        end
    end

    local count = 0
    for _, obj in pairs(targets) do
        count = count + 1
        obj:effect(107)
        obj:buff(sp, 2, me)
        if me ~= obj and obj:is(OBJECT_TYPE.CHARACTER) then
            obj:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(sp:name())))
        end
    end

    if buff:time() % 2 == 0 then
        me:effect(115)
    end

    if count > 0 then
        me:sound(108)
    end
end
