-- item: 산신의보물지도

local quest = require('lib.quest')

return {
    on_activated = function(me, item)
        local spot = quest.mountain_treasure_spot(me)
        if spot == 'got' then
            me:dialog(item:model(), '이미 산신의비단을 찾았다.', { prev = false, next = true })
        elseif spot ~= nil then
            me:dialog(item:model(), string.format('%s %d, %d에 숨겨두다.', spot.map, spot.x, spot.y),
                { prev = false, next = true })
        end
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
