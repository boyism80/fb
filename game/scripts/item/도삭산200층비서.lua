-- 도삭산200층비서 사용 스크립트
function ON_ACTIVE_4011(me, item)
    me:rmitem(item, 1, ITEM_DELETE_TYPE_REDUCE)
    me:map('도삭산200층주막')
end