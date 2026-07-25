-- spell: 귀환
-- Return spell: warp to random tavern by nation
local server = require('lib.server')

return {
    ON_CAST = function(me, spell)
        local sound = 33
        local mp = 30
        local error = me:assert(STATE.GHOST, STATE.RIDING)
        if error ~= nil then
            return me:message(error)
        end

        if me:mp() < mp then
            me:message('마력이 부족합니다.')
            return false
        end
        me:mp_down(mp)

        if server.warp_to_return_map(me) ~= nil then
            return
        end

        me:sound(sound)
        me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
        me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
