-- 도삭산800층비서 사용 스크립트
function ON_ACTIVE_4017(me, item)
	me:rmitem(item, 1, ITEM_DELETE_TYPE_REDUCE)
	me:map('도삭산800층주막')
end