-- 도토리 사용 스크립트
function ON_ACTIVE_3014(me, item)
    me:hp(math.min(me:hp()+8, me:maxhp()))
end