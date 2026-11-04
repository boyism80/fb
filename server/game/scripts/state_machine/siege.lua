local siege = require('schedule.siege')

local REFRESH_MS = 30000

return {
    on_init = function(sm)
        sm:every('refresh', REFRESH_MS, 'on_refresh')
        if siege.has_heart_map() == false then
            return
        end
        sm:every('tick', 1000, 'on_tick')
    end,

    on_refresh = function(sm)
        refresh_castles()
    end,

    on_tick = function(sm)
        siege.tick(sm)
    end,
}
