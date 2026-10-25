-- spell: 귀환
-- Return spell: warp to random tavern by nation
local server = require('lib.server')

return {
    on_cast = function(me, spell)
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

        if server.warp_to_return_map(me) then
            return
        end

        me:mp(me:mp() + mp)
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
