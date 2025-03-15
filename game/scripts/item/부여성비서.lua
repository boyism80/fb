-- 부여성비서 사용 스크립트
function ON_ACTIVE_4003(me, item)
    local maps = maps = {'주막연실이네', '주막연실언니네', '주막연실이모네'}


    math.randomseed(seed())
    local i = math.random(1, #maps)
    local map = maps[i]

    me:rmitem(item, 1, ITEM_DELETE_TYPE_REDUCE)
    me:map(map)
end