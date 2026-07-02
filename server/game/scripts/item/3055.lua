-- item: 포도

-- 포도 사용 스크립트
function ON_ACTIVATED_3055(me, item)
    me:hp(math.min(me:hp()+5, me:maxhp()))
end
