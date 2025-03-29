-- 도삭산700층비서 사용 스크립트
function ON_ACTIVE_4016(me, item)
	if me:map('도삭산700층주막') then
        me:rmitem(item, 1, ITEM_DELETE_TYPE_REDUCE)
    end
end