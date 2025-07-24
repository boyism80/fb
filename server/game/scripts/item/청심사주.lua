-- 청심사주 사용 스크립트
function ON_ACTIVE_2210(me, item)
    me:hp(math.min(me:hp()+150, me:maxhp()))
    me:mp(math.min(me:mp()+100, me:maxmp()))
end