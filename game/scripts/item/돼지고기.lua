-- 돼지고기 사용 스크립트
function ON_ACTIVE_3017(me, item)
    me:hp(math.min(me:hp()+62, me:maxhp()))
end