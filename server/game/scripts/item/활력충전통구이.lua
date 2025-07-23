-- 활력충전통구이 사용 스크립트
function ON_ACTIVE_3050(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end