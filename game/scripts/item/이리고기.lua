-- 이리고기 사용 스크립트
function ON_ACTIVE_3043(me, item)
    me:hp(math.min(me:hp()+78, me:maxhp()))
end