-- map: 동지집

local function warp_same(me, x, y)
    local map = me:map()
    if map == nil then
        return
    end
    me:map(map, x, y)
end

return {
    -- Warp tiles x=25..28, y=31
    on_enter_warehouse = function(me)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local x, y = me:position()
        local sel, list_btn = me:list(nil, "창고에 들어가시겠습니까?", {
            "창고에 들어간다.",
            "나중에...",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end

        if sel == 1 then
            if me:has_items("동지창고열쇠", 1) then
                if me:rmitem("동지창고열쇠", 1, ITEM_DELETE_TYPE.GIVE) then
                    me:message("동지창고열쇠 사용합니다.", MESSAGE_TYPE.NOTIFY)
                    warp_same(me, x, y - 5)
                else
                    warp_same(me, x, y + 2)
                end
            else
                warp_same(me, x, y + 2)
            end
        else
            warp_same(me, x, y + 2)
        end
    end,

    -- Warp tiles x=25..28, y=28
    on_exit_warehouse = function(me)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local x, y = me:position()
        local sel, list_btn = me:list(nil, "창고에서 퇴장하시겠습니까?", {
            "나가겠습니다.",
            "나중에...",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end

        if sel == 1 then
            warp_same(me, x, y + 5)
        else
            warp_same(me, x, y - 2)
        end
    end,
}
