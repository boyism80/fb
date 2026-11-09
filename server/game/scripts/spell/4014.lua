-- spell: 부활
-- 부활 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        if not you:is(OBJECT_TYPE.LIFE) then
            me:message('걸리지 않습니다.')
            return false
        end

        local error = nil
        if me == you then
            error = me:assert(STATE.RIDING)
        else
            error = me:assert(STATE.RIDING, STATE.GHOST)
        end
        if error ~= nil then
            return me:message(error)
        end
        if not spell.cast(me, you, sp, { mp = 300, sound = 8, effect = 11, no_assert = true }) then
            return
        end

        if you:is(OBJECT_TYPE.CHARACTER) then
            you:revive()
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
