-- 쥐고기 사용 스크립트
function ON_ACTIVE_3019(me, item)
    me:hp(math.min(me:hp()+28, me:maxhp()))
end