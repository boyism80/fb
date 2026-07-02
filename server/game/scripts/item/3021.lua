-- item: 여우고기

-- 여우고기 사용 스크립트
function ON_ACTIVATED_3021(me, item)
    me:hp(math.min(me:hp()+18, me:maxhp()))
end
