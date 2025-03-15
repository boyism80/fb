-- 구기자 사용 스크립트
function ON_ACTIVE_3101(me, item)
    me:hp(math.min(me:hp()+300, me:maxhp()))
end