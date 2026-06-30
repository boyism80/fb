-- item: 신부여성비서
-- 신부여성비서 사용 스크립트
function ON_ACTIVATED_4025(me, item)
	local maps = {'신부여성주막', '신부여서쪽주막', '신부여서쪽주막2'}

    math.randomseed(seed())
    local i = math.random(1, #maps)
    local map = maps[i]

    if me:map(map) then
        me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    end
end
