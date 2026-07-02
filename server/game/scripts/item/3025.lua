-- item: 호랑이고기

-- 호랑이고기 사용 스크립트
function ON_ACTIVATED_3025(me, item)
    me:hp(math.min(me:hp()+32, me:maxhp()))
end
