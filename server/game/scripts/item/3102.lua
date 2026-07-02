-- item: 비둘기고기

-- 비둘기고기 사용 스크립트
function ON_ACTIVATED_3102(me, item)
    me:hp(math.min(me:hp()+150, me:maxhp()))
end
