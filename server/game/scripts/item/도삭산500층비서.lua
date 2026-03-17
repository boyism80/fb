-- 도삭산500층비서 사용 스크립트
function ON_ACTIVATED_4014(me, item)
    if me:map('도삭산500층주막') then
        me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    end
end