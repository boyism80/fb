-- 장안성비서 사용 스크립트
function ON_ACTIVE_4002(me, item)
    if me:map('장안성주막') then
        me:rmitem(item, 1, ITEM_DELETE_TYPE_REDUCE)
    end
end