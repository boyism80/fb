-- item: 청심사주

-- 청심사주 사용 스크립트
function ON_ACTIVATED_2210(me, item)
    me:hp(math.min(me:hp()+150, me:maxhp()), false)
    me:mp(math.min(me:mp()+100, me:maxmp()), false)
    me:update()
end
