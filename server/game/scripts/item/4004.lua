-- item: 국내성비서
-- 국내성비서 사용 스크립트
function ON_ACTIVATED_4004(me, item)
	local maps = {'주막왈숙네', '왈숙언니네', '왈숙이모네'}

    math.randomseed(seed())
    local i = math.random(1, #maps)
    local map = maps[i]

    if me:map(map) then
        me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    end
end
