-- 도삭산900층비서 사용 스크립트
function ON_ACTIVE_4018(me, item)
    me:map('도삭산900층주막')
    me:rmitem(item, 1, ITEM_DELETE_TYPE_REDUCE)
end