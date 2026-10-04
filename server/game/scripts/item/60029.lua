-- item: 신국내성비서

return {
    on_activated = function(me, item)
        local maps = {
            '신국내북서주막',
            '신국내북주막',
            '신국내북동주막',
            '신국내남서주막',
            '신국내남주막',
            '신국내남동주막',
        }

        math.randomseed(seed())
        local i = math.random(1, #maps)
        local map = maps[i]

        local name = item:model():name()
        if not me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE) then
            return
        end
        if me:map(map) ~= true then
            me:mkitem(name, 1)
        end
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
