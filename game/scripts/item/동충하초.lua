-- 동충하초 사용 스크립트
function ON_ACTIVE_3103(me, item)
    me:hp(math.min(me:hp()+15, me:maxhp()))
end