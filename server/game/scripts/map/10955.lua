-- map: 세시밭 (유두절 콩/조/피 ground-item spawn)
local crops = require('lib.sesibat_crops')

return {
    on_map_init = crops.start,
}
