-- map: 세시마을
local festival = require('lib.festival')
local chilsung = require('lib.chilsung_entrance')

-- Well tiles near 단오집 (머리감기 warp).
local WELL = {
    ['38,3'] = true, ['38,4'] = true, ['38,5'] = true, ['38,6'] = true,
    ['39,3'] = true, ['39,6'] = true, ['39,7'] = true,
    ['40,2'] = true, ['40,7'] = true,
    ['41,2'] = true,
    ['42,2'] = true, ['42,7'] = true,
    ['43,3'] = true, ['43,6'] = true,
}

return {
    -- Warp dest script for 칠성당 entrance tiles (59,42)/(60,42).
    on_enter_chilsungdang = chilsung.try_enter,

    on_map_move = function(me)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end
        if not festival.is('단오') then
            return
        end

        local x, y = me:position()
        if WELL[string.format('%d,%d', x, y)] ~= true then
            return
        end
        festival.wash_hair(me)
    end,
}
