-- map: 칠성당
local yudu_gosa = require('lib.yudu_gosa')
local magpie_bridge = require('lib.magpie_bridge')

local function boot()
    -- on_map_init runs only once per process map load; enter must set the timer again.
    magpie_bridge.set_timer()
end

return {
    on_map_init = function(map)
        yudu_gosa.start(map)
        boot()
    end,
    on_map_enter = function(me)
        boot()
    end,
}
