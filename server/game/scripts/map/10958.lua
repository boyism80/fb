-- map: 견우의집
local magpie_bridge = require('lib.magpie_bridge')

return {
    on_map_enter = magpie_bridge.sync_timer,
}
