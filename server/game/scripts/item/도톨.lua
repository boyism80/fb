-- 도톨 사용 스크립트
function ON_ACTIVE_9(me, item)
    me:hp(math.min(me:hp()+8, me:maxhp()))
end