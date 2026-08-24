-- map: 세시호수

local festival = require('lib.festival')

local TILE_MIN = 10238
local TILE_MAX = 10240

return {
    on_map_move = function(me)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end
        if not festival.is('칠석') then
            return
        end

        local map = me:map()
        if map == nil then
            return
        end

        local x, y = me:position()
        local tile_id = map:tile(x, y)
        if tile_id == nil or tile_id < TILE_MIN or tile_id > TILE_MAX then
            return
        end

        if math.random(1, 10) ~= 1 then
            return
        end

        if me:has_items('잉어', 1) then
            return
        end

        local weapon = me:weapon()
        if weapon == nil or weapon:name() ~= '그물' then
            me:dialog(nil, '앗... 잉어를 발견했는데 그물이 없어서 놓쳐버렸다!', { prev = false, next = false })
            return
        end

        if me:mkitem('잉어', 1) == nil then
            return
        end
        me:message('잉어 잡았습니다.', MESSAGE_TYPE.STATE)
        me:dialog(nil, '드디어 잉어를 잡았다! 정말 구하기 힘들구나.', { prev = false, next = false })
    end,
}
