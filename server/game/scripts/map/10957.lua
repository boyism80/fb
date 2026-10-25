-- map: 오작교 (Magpie Bridge)
local magpie_bridge = require('lib.magpie_bridge')

return {
    on_map_move = magpie_bridge.on_step,
    on_map_enter = magpie_bridge.on_bridge_enter,
    on_map_leave = magpie_bridge.on_bridge_leave,
}
