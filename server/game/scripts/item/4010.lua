-- item: 도삭산100층비서

return {
    on_activated = function(me, item)
        local name = item:model():name()
        if not me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE) then
            return
        end
        if me:map('도삭산100층주막') ~= true then
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
