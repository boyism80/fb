-- 노회 사용 스크립트
function ON_ACTIVE_3080(me, item)
    me:hp(math.min(me:hp()+30, me:maxhp()))
end