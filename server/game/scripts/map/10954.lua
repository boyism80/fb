-- map: 칠성당
local yudu_gosa = require('lib.yudu_gosa')

return {
    on_map_init = function(map)
        yudu_gosa.start(map)
    end,
}
