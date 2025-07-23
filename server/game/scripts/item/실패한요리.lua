-- 실패한요리 사용 스크립트
function ON_ACTIVE_3068(me, item)
    me:hp(math.min(me:hp()+5, me:maxhp()))
end