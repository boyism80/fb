-- spell: 노획
-- 노획 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local map = me:map()
        if map == nil then
            return
        end

        local mp = 30
        local effect = nil
        local sound = 37
        if not spell.cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            return
        end

        local x, y = me:front_position()
        local items = map:belows(x, y, OBJECT_TYPE.ITEM)
        me:gain(table.unpack(items))
        me:chat('노획', CHAT_TYPE.BLUE, false)
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
