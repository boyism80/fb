-- 좋은뱀고기 사용 스크립트
function ON_ACTIVE_3049(me, item)
    me:hp(math.min(me:hp()+100, me:maxhp()))
end