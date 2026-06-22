-- spell: 신령지익
-- 신령지익 캐스팅
local spell = require('lib.spell')

function ON_CAST_4016(me, sp)
    local mp = 0
    local sound = 109
    local effect = 116
    local buff_time = 45
    if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
        me:buff(sp, buff_time, me)
    end
end

-- 신령지익 버프 효과
function ON_BUFF_4016(me, sp)

end

-- 신령지익 버프 해제 효과
function ON_UNBUFF_4016(me, sp)

end

-- 신령지익 지속 효과
function ON_CONCAST_4016(me, caster, buff)
    local x, y = me:position()
    local begin_x = x-3
    local begin_y = y-3
    local end_x = x+3
    local end_y = y+3
    local targets = {}
    local sp = name2spell('신령지익진')
    for _, obj in pairs(me:nears(OBJECT_TYPE.LIFE)) do
        local obj_x, obj_y = obj:position()
        if not obj:isbuff(sp) and obj_x >= begin_x and obj_x <= end_x and obj_y >= begin_y and obj_y <= end_y then
            table.insert(targets, obj)
        end
    end

    local count = 0
    for _, obj in pairs(targets) do
        count = count + 1
        obj:effect(117)
        obj:buff(sp, 2, me)
        if me ~= obj and obj:is(OBJECT_TYPE.CHARACTER) then
            obj:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(sp:name())))
        end
    end

    if buff:time() % 2 == 0 then
        me:effect(116)
    end

    if count > 0 then
        me:sound(109)
    end
end
