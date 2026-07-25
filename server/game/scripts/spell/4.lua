-- spell: 소환
-- 소환 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp, name)
        if me:name() == name then
            me:message('나 자신을 소환할 수 없습니다.')
            return
        end

        local map = me:map()
        if map == nil then
            me:message('소환할 수 없는 공간입니다.')
            return
        end

        local x, y = me:position()
        local direction = me:direction()
        local my_level = me:level()
        local ch = name2ch(name)
        if ch == nil then
            me:message(string.format('%s님은 현재 바람의나라에 없습니다.', name))
            return
        end
        if ch:level() > my_level then
            me:message('마력이 미치지 않습니다.')
            return
        end

        local sound = 36
        local effect = 3
        local mp = 30
        if spell.cast(me, nil, nil, {mp=mp}) then
            local new_x, new_y = spell.TELEPORT_LOOKUP(me, map, x, y, direction)
            ch:map(map, new_x, new_y)
            me:action(ACTION.CAST_SPELL, DURATION.SPELL)
            me:sound(sound)
            me:effect(effect)
            me:message(string.format('%s 외웠습니다.', name_with(sp:name())))
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
