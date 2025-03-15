-- 영양통구이 사용 스크립트
function ON_ACTIVE_3062(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end