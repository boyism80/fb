-- 환상의도시락 사용 스크립트
function ON_ACTIVE_3054(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end