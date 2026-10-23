-- map: 동지집

local function warehouse_door(me, door_y)
    local map = me:map()
    if map == nil or map:id() ~= 10953 then
        return nil
    end

    local x, y = me:position()
    if y ~= door_y or x < 25 or x > 28 then
        return nil
    end
    return map, x, y
end

return {
    -- Warp tiles x=25..28, y=31
    on_enter_warehouse = function(me)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local sel, list_btn = me:list(me, "창고에 들어가시겠습니까?", {
            "창고에 들어간다.",
            "나중에...",
        }, { prev = false })

        local map, x, y = warehouse_door(me, 31)
        if map == nil then
            return
        end

        if list_btn ~= DIALOG_RESULT.QUIT and sel == 1 and me:rmitem("동지창고열쇠", 1, ITEM_DELETE_TYPE.GIVE) then
            me:message("동지창고열쇠 사용합니다.", MESSAGE_TYPE.STATE)
            if me:map(map, x, y - 5) ~= true then
                me:mkitem("동지창고열쇠", 1)
            end
        else
            me:map(map, x, y + 2)
        end
    end,

    -- Warp tiles x=25..28, y=28
    on_exit_warehouse = function(me)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local sel, list_btn = me:list(nil, "창고에서 퇴장하시겠습니까?", {
            "나가겠습니다.",
            "나중에...",
        }, { prev = false })

        local map, x, y = warehouse_door(me, 28)
        if map == nil then
            return
        end

        if list_btn ~= DIALOG_RESULT.QUIT and sel == 1 then
            me:map(map, x, y + 5)
        else
            me:map(map, x, y - 2)
        end
    end,
}
