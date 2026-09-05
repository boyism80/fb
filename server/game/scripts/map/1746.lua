-- map: 고구려-고균도선착장
local ferry_boat = require('lib.ferry_boat')

return {
    on_map_init = ferry_boat.start,
}
