-- 도삭산100층비서 사용 스크립트
function ON_ACTIVE_4010(me, item)
    me:rmitem(item, 1, ITEM_DELETE_TYPE_REDUCE)
    me:map('도삭산100층주막')
end