-- map: 장안성감옥

return {
    -- on_map_init = function(me)
    -- end,

    -- on_map_move = function(me)
    -- end,

    -- on_map_enter = function(me)
    -- end,

    -- on_map_leave = function(me)
    -- end,

    on_secret_passage = function(me)
        local dest = name2map("장안성")
        if dest == nil then
            return
        end

        math.randomseed(seed())
        local position = { math.random(125, 126), math.random(41, 42) }
        if not me:rmitem("감옥비밀통로열쇠", 1, ITEM_DELETE_TYPE.NONE) then
            return
        end
        if me:map(dest, position) ~= true then
            me:mkitem("감옥비밀통로열쇠", 1)
        end
    end,
}
