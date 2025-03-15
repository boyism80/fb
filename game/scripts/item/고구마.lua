-- 고구마 사용 스크립트
function ON_ACTIVE_3045(me, item)
    me:hp(math.min(me:hp()+100, me:maxhp()))
end