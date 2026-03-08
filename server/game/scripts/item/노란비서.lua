-- Yellow Note use script
function ON_ACTIVE_4000(me, item)
    if warp_to_return_map(me) then
        me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    end
end
