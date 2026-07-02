-- item: 뱀고기

-- 뱀고기 사용 스크립트
function ON_ACTIVATED_3020(me, item)
    me:hp(math.min(me:hp()+54, me:maxhp()))
end
