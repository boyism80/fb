-- item: 초코바나나

-- 초코바나나 사용 스크립트
function ON_ACTIVATED_3000(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end
