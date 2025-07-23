-- 곰가죽 사용 스크립트
function ON_ACTIVE_3023(me, item)
    me:hp(math.min(me:hp()+18, me:maxhp()))
end