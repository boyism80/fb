-- 생장촉진반찬 사용 스크립트
function ON_ACTIVE_3010(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end