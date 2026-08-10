-- map: 세시밭 (유두절 피/콩/조 줍기 via tile object)
local festival = require('lib.festival')

-- tile.object ranges from live investigation
local CROPS = {
    { min = 7905, max = 7907, name = '콩' },
    { min = 7634, max = 7635, name = '조' },
    { min = 7639, max = 7640, name = '피' },
}

return {
    on_map_move = function(me)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end
        if not festival.is('유두') then
            return
        end

        local map = me:map()
        if map == nil then
            return
        end

        local x, y = me:position()
        local _, obj = map:tile(x, y)
        if obj == nil then
            return
        end

        local crop_name = nil
        for _, crop in ipairs(CROPS) do
            if obj >= crop.min and obj <= crop.max then
                crop_name = crop.name
                break
            end
        end
        if crop_name == nil then
            return
        end

        -- Same roll style as 세시호수 잉어 (10%).
        if math.random(1, 10) ~= 1 then
            return
        end

        if me:mkitem(crop_name, 1) == nil then
            return
        end
        me:message(crop_name .. ' 주웠습니다.', MESSAGE_TYPE.NOTIFY)
    end,
}
