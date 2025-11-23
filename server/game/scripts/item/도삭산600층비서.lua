-- 도삭산600층비서 사용 스크립트
function ON_ACTIVE_4015(me, item)
    if me:map('도삭산600층주막') then
        me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    end
end