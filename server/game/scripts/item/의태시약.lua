local spell = require('lib.spell')

function ON_ACTIVATED_9050(me, item)
    local front = spell.front_exclude_item(me)
    if front == nil then
        return
    end

    me:cast(front, '의태')
end

function ON_DEACTIVATED_9050(me, item)
end
