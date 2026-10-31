-- map: castle occupancy gate
local castle_lib = require('lib.castle')

return {
    on_map_enter = function(me)
        castle_lib.evict(me)
    end,
}
