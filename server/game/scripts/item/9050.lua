-- item: 의태시약

local spell = require('lib.spell')

-- 의태시약 사용 스크립트
function ON_ACTIVATED_9050(me, item)
    local front = spell.front_exclude_item(me)
    if front == nil then
        return
    end

    me:cast(front, '의태')
end

-- 의태시약 해제 스크립트
function ON_DEACTIVATED_9050(me, item)
end
