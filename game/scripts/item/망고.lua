-- 망고 사용 스크립트
function ON_ACTIVE_3076(me, item)
    me:hp(math.min(me:hp()+5, me:maxhp()))
end