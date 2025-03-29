-- 도삭산600층비서 사용 스크립트
function ON_ACTIVE_4015(me, item)
    me:map('도삭산600층주막')
    me:rmitem(item, 1, ITEM_DELETE_TYPE_REDUCE)
end