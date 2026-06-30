-- spell: 출두
-- 출두 캐스팅
local spell = require('lib.spell')

function ON_CAST_3(me, sp, name)
    if me:name() == name then
        return me:message('나 자신을 출두할 수 없습니다.')
    end
    local my_level = me:level()
    local ch = name2ch(name)
    if ch == nil then
        return me:message(string.format('%s님은 현재 바람의나라에 없습니다.', name))
    end
    if ch:level() > my_level then
        return me:message('마력이 미치지 않습니다.')
    end

    local map = ch:map()
    if map == nil then
        return me:message('이동할 수 없는 공간입니다.')
    end

    local sound = 29
    local effect = 16
    local mp = 30
    if spell.cast(me, nil, nil, {mp=mp, default_action=false}) then
        local x, y = ch:position()
        local direction = me:direction()
        local new_x, new_y, direction = spell.TELEPORT_LOOKUP(me, map, x, y, direction)
        me:map(map, new_x, new_y)
        me:script('scripts/spell/3.lua', 'ON_CAST_3_BULK', sp:name(), map:model():name(), {new_x, new_y}, direction, sound, effect)
    end
end

function ON_CAST_3_BULK(me, sp, map, position, direction, sound, effect)
    me:direction(direction)
    me:sound(sound)
    me:effect(effect)
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(sp)))
end
