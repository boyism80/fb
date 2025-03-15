-- 도삭산300층비서 사용 스크립트
function ON_ACTIVE_4012(me, item)
    me:rmitem(item, 1, ITEM_DELETE_TYPE_REDUCE)
    me:map('도삭산300층주막')
end