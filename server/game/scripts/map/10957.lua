-- map: 오작교 (Magpie Bridge)
local magpie_bridge = require('lib.magpie_bridge')

return {
    on_map_init = magpie_bridge.start,
    on_map_move = magpie_bridge.on_step,
    on_map_enter = magpie_bridge.on_bridge_enter,
}
