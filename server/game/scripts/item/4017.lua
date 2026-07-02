-- item: 도삭산800층비서

-- 도삭산800층비서 사용 스크립트
function ON_ACTIVATED_4017(me, item)
	if me:map('도삭산800층주막') then
        me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    end
end
