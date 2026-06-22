-- item: 노란비서
-- Yellow Note use script
local server = require('lib.server')

function ON_ACTIVATED_4000(me, item)
    if server.warp_to_return_map(me) then
        me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    end
end
