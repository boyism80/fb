-- 초코바나나 사용 스크립트
function ON_ACTIVE_3000(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end