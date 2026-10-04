-- item: 신부여성비서

return {
    on_activated = function(me, item)
    	local maps = {'신부여성주막', '신부여서쪽주막', '신부여서쪽주막2'}

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
