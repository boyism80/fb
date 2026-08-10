-- item: 채집통 [8 마리]
local jar = require('lib.butterfly_jar')

return {
    on_activated = function(me, item)
        jar.shake(me, item)
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
